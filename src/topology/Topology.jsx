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

import { useContext, useState } from "react";
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
    DrawerContentBody,
    useInterval
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
        physics: { stabilization: { enabled: false } },
        layout: { improvedLayout: true },
        edges: {
            color: EdgesColor,
            arrows: { to: false }
        },
        height: '100%',
        width: '100%'
    };

    const getGraph = () => {
        // only make a dbus request if the service is active
        if (active !== true) {
            if (loading) {
                setLoading(false);
            }
            return;
        }

        const dbusClient = cockpit.dbus("com.silabs.Wisun.BorderRouter", { bus: "system" });

        dbusClient.wait(() => {
            const proxy = dbusClient.proxy();

            proxy.wait().then(() => {
                if (proxy.valid === false) {
                    setHasError(true);
                    setLoading(false);
                } else if (proxy.WisunMode !== undefined) {
                    const nodes = [];
                    const edges = [];
                    let borderRouterEUI64;

                    // get the border router's device eui for LFN
                    for (let i = 0; i < proxy.Nodes.length; i++) {
                        if (proxy.Nodes[i][1].ipv6.v.length === 2 &&
                            proxy.Nodes[i][1].node_role?.v === NodeRoles.BorderRouter) {
                            borderRouterEUI64 = base64ToHex(proxy.Nodes[i][0]);
                            break;
                        }
                    }

                    for (let i = 0; i < proxy.Nodes.length; i++) {
                        if (proxy.Nodes[i][1].ipv6.v.length === 2) {
                            if (proxy.Nodes[i][1].parent === undefined &&
                                (proxy.Nodes[i][1].is_border_router === undefined &&
                                    proxy.Nodes[i][1].node_role?.v !== NodeRoles.LFN)) {
                                continue;
                            }

                            const eui64 = base64ToHex(proxy.Nodes[i][0]);
                            const ipv6 = base64ToHex(proxy.Nodes[i][1].ipv6.v[1]);
                            const nodeRole = proxy.Nodes[i][1].node_role?.v ?? NodeRoles.FFN;

                            // for safety in case of unsupported node
                            if (nodeRole > 2 || nodeRole < 0) {
                                continue;
                            }

                            let parentEUI64;

                            if (proxy.Nodes[i][1].parent !== undefined) {
                                parentEUI64 = base64ToHex(proxy.Nodes[i][1].parent.v);
                            } else if (nodeRole === NodeRoles.LFN) {
                                parentEUI64 = borderRouterEUI64;
                            }

                            nodes.push({
                                id: eui64,
                                label: eui64.slice(eui64.length - 4),
                                color: NodeRolesColors[nodeRole],
                                ipv6: beautifyIpv6String(ipv6),
                                parentEUI64,
                                nodeRole,
                                shape: nodeRole === NodeRoles.BorderRouter ? "box" : "ellipse",
                                font: nodeRole === NodeRoles.BorderRouter ? "18px arial black" : "14px arial black"
                            });
                            edges.push({
                                id: eui64,
                                from: eui64,
                                to: parentEUI64,
                                dashes: nodeRole === NodeRoles.LFN,
                                width: 2
                            });
                        }
                    }

                    setStateGraph({
                        nodes,
                        edges
                    });

                    if (autoZoom && network !== undefined) {
                        network.fit({
                            animation: {
                                duration: 1000,
                                easingFunction: "linear",
                            }
                        });
                    }

                    setLoading(false);
                }

                dbusClient.close();
            });
        });
    };

    useInterval(getGraph, 1000);

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
                            <DescriptionListTerm>Total:</DescriptionListTerm>
                            <DescriptionListDescription>{stateGraph.nodes.length}</DescriptionListDescription>
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
