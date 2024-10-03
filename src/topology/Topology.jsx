/*
 * Copyright (c) 2022-2023 Silicon Laboratories Inc. (www.silabs.com)
 *
 * Cockpit is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Cockpit is distributed in the hope that it will be useful, but&
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Cockpit; If not, see <http://www.gnu.org/licenses/>.
 */

import { useEffect, useContext, useState } from "react";
import cockpit from "cockpit";
import { AppContext } from "../app";
import Graph from "react-graph-vis";
import {
    Alert,
    Button,
    Checkbox,
    DescriptionList,
    DescriptionListDescription,
    DescriptionListGroup,
    DescriptionListTerm,
    Drawer,
    DrawerContent,
    DrawerContentBody
} from "@patternfly/react-core";
import { BarsIcon } from '@patternfly/react-icons';
import CenteredContent from "../utils/CenteredContent";
import Loading from "../utils/Loading";
import TopologyDrawerContent from "./TopologyDrawerContent";
import { base64ToHex, beautifyIpv6String } from "../utils/functions";

const _ = cockpit.gettext;

export const NodeRoles = Object.freeze({
    BorderRouter: 0,
    FFN: 1,
    LFN: 2
});

const NodeRolesColors = ["#d91e2a", "#00b970", "#fad54c"];
const EdgesColor = "#0f62fe";
let RoutingGraphindexedByIpv6 = [];
let borderrouterIpv6;
let dbusClient;
const Topology = () => {
    const [loading, setLoading] = useState(true);
    const [stateGraph, setStateGraph] = useState({
        nodes: [],
        edges: []
    });
    const [hasError, setHasError] = useState(false);
    const [isExpanded, setIsExpanded] = useState(false);
    const [network, setNetwork] = useState(undefined);
    const [selectedNode, setSelectedNode] = useState(null);
    const [autoZoom, setAutoZoom] = useState(true);

    const { active } = useContext(AppContext);

    const options = {
        physics: { stabilization : { enabled: false } },
        layout: {
            improvedLayout: true,
            hierarchical: {
                enabled: true, // Enable hierarchical layout
                direction: 'UD', // Direction of the layout: 'UD' (Up-Down), 'LR' (Left-Right), etc.
                sortMethod: 'hubsize', // Use hubsize to sort nodes based on connectivity
                blockShifting: false, // Enable block shifting to optimize the layout
                edgeMinimization: true, // Minimize edge crossings
                parentCentralization: true, // Centralize parent nodes relative to their children
            },
        },
        edges: {
            color: EdgesColor,
            arrows: { to: false }
        },
        height: '100%',
        width: '100%'
    };
    let node_level;
    const checkLinkToBR = (id) => {
        if (RoutingGraphindexedByIpv6[id][2].length === 0 && RoutingGraphindexedByIpv6[id][0] === borderrouterIpv6) {
            return true;
        } else if (RoutingGraphindexedByIpv6[id][2].length === 0 &&
            RoutingGraphindexedByIpv6[id][0] !== borderrouterIpv6) {
            return false;
        } else if (RoutingGraphindexedByIpv6[id][2] !== 0 &&
            RoutingGraphindexedByIpv6[RoutingGraphindexedByIpv6[id][2][0]] !== undefined) {
            if (checkLinkToBR(RoutingGraphindexedByIpv6[id][2][0])) {
                node_level = node_level + 1;
                return true;
            }
        } else {
            return false;
        }
    };

    const initializeDbus = () => {
        // only make a dbus request if the service is active
        if (active !== true) {
            if (loading) {
                setLoading(false);
            }
            return;
        }
        dbusClient = cockpit.dbus("com.silabs.Wisun.BorderRouter", { bus: "system" });

        dbusClient.wait(() => {
            const proxy = dbusClient.proxy();
            const proxy_signal = dbusClient.proxy("org.freedesktop.DBus.Properties", "/com/silabs/Wisun/BorderRouter");
            proxy.wait().then(() => {
                if (proxy.valid === false) {
                    setHasError(true);
                    setLoading(false);
                } else if (proxy.WisunMode !== undefined) {
                    // the signal
                    proxy_signal.addEventListener("signal", (event, name, args) => {
                        if (args[2][0] === "RoutingGraph") {
                            setTimeout(() => {
                                processGraphData(proxy);
                            }, 3000);
                        }
                    });

                    // Initial graph processing
                    processGraphData(proxy);
                }
            });
        });
    };

    const processGraphData = (proxy) => {
        const nodes = [];
        const edges = [];
        // Transform the array into an object indexed by 'id'
        RoutingGraphindexedByIpv6 = proxy.RoutingGraph.reduce((acc, item) => {
            acc[item[0]] = item;
            return acc;
        }, {});
        for (let i = 0; i < proxy.RoutingGraph.length; i++) {
            const ipv6 = base64ToHex(proxy.RoutingGraph[i][0]);
            let nodeRole;
            if (proxy.RoutingGraph[i][1] === false && proxy.RoutingGraph[i][2].length === 0) {
                nodeRole = NodeRoles.BorderRouter;
                borderrouterIpv6 = proxy.RoutingGraph[i][0];
            } else if (proxy.RoutingGraph[i][1] === true) {
                nodeRole = NodeRoles.LFN;
            } else {
                nodeRole = NodeRoles.FFN;
            }

            let parentIPv6;
            node_level = 0;
            if (checkLinkToBR(proxy.RoutingGraph[i][0])) {
                if (proxy.RoutingGraph[i][2][0] !== undefined) {
                    parentIPv6 = base64ToHex(proxy.RoutingGraph[i][2][0]);
                    nodes.push({
                        id: ipv6,
                        label: ipv6.slice(ipv6.length - 4),
                        color: NodeRolesColors[nodeRole],
                        ipv6: beautifyIpv6String(ipv6),
                        parentIPv6: beautifyIpv6String(parentIPv6),
                        nodeRole,
                        level: node_level,
                        shape: nodeRole === NodeRoles.BorderRouter ? "box" : "ellipse",
                        font: nodeRole === NodeRoles.BorderRouter ? "18px arial black" : "14px arial black"
                    });
                    edges.push({
                        id: ipv6,
                        from: ipv6,
                        to: parentIPv6,
                        dashes: nodeRole === NodeRoles.LFN,
                        width: 2
                    });
                } else {
                    nodes.push({
                        id: ipv6,
                        label: ipv6.slice(ipv6.length - 4),
                        color: NodeRolesColors[nodeRole],
                        ipv6: beautifyIpv6String(ipv6),
                        nodeRole,
                        level: node_level,
                        shape: nodeRole === NodeRoles.BorderRouter ? "box" : "ellipse",
                        font: nodeRole === NodeRoles.BorderRouter ? "18px arial black" : "14px arial black"
                    });
                    edges.push({
                        id: ipv6,
                        from: ipv6,
                        to: parentIPv6,
                        dashes: nodeRole === NodeRoles.LFN,
                        width: 2
                    });
                }
            }
        }

        setStateGraph({
            nodes,
            edges
        });

        setLoading(false);
    };

    useEffect(() => {
        initializeDbus();
        return () => {
            dbusClient.close();
        };
    });

    useEffect(() => {
        if (autoZoom && network !== undefined) {
            network.fit({
                animation: {
                    duration: 1000,
                    easingFunction: "linear",
                }
            });
        }
    }, [autoZoom, network]);

    const closeDrawer = () => {
        network.unselectAll();
        setIsExpanded(false);
        setSelectedNode(null);
    };

    if (loading) {
        return (
            <Loading />
        );
    }

    if (hasError || active === null) {
        return (
            <CenteredContent>
                <Alert variant='danger' title="Could not retrieve network topology" />
            </CenteredContent>
        );
    }

    if (active === false) {
        return (
            <CenteredContent>
                <Alert variant="info" title="Start WSBRD to view the network topology" />
            </CenteredContent>
        );
    }

    const events = {
        selectNode: (event) => {
            const { nodes } = event;
            const graphNode = stateGraph.nodes.find((n) => n.id.localeCompare(nodes[0]) === 0);

            if (graphNode !== undefined && network !== undefined) {
                setIsExpanded(true);
                setSelectedNode(graphNode);
            }
        },
        deselectNode: () => {
            setIsExpanded(false);
            setSelectedNode(null);
        },
        dragStart: () => {
            setAutoZoom(false);
        },
        zoom: () => {
            setAutoZoom(false);
        }
    };

    return (
        <Drawer isExpanded={isExpanded} position="right" onExpand={() => setIsExpanded(true)} style={{ height: '98%' }}>
            <DrawerContent
                panelContent={
                    <TopologyDrawerContent
                        selectedNode={selectedNode}
                        closeDrawer={closeDrawer}
                        graph={stateGraph}
                    />
                }
            >
                <DrawerContentBody style={{ position: 'relative' }}>
                    <Graph
                        graph={stateGraph} options={options} events={events} getNetwork={(n) => setNetwork(n)}
                    />
                    <Button
                        variant="primary" icon={<BarsIcon />} onClick={() => setIsExpanded(true)} style={
                            {
                                position: 'absolute',
                                top: '20px',
                                right: '20px',
                            }
                        }
                    >
                        Details
                    </Button>
                    <DescriptionList
                        style={{
                            position: 'absolute',
                            bottom: '20px',
                            left: '10px',
                        }} isHorizontal isFluid isCompact
                    >
                        <DescriptionListGroup>
                            <DescriptionListTerm>Number of nodes:</DescriptionListTerm>
                            <DescriptionListDescription>{stateGraph.nodes.length - 1}</DescriptionListDescription>
                        </DescriptionListGroup>
                        <Checkbox
                            id="auto-zoom-check"
                            label="Auto Zoom"
                            reversed
                            isChecked={autoZoom}
                            onChange={() => setAutoZoom(!autoZoom)}
                        />
                    </DescriptionList>
                </DrawerContentBody>
            </DrawerContent>
        </Drawer>
    );
};

export default Topology;
