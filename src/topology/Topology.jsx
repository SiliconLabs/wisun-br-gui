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

import {
    useEffect,
    useContext,
    useState,
    useCallback,
    useRef
} from "react";
import cockpit from "cockpit";
import { AppContext, SERVICE_SHORT_NAMES } from "../app"; // Added: reuse shared service metadata
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

const Topology = () => {
    const dbusClient = useRef(null); // Moved inside the component
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

    // Added: include the selected service before loading
    const { active, selectedService, serviceDbus } = useContext(AppContext);
    const selectedServiceName = selectedService
        ? SERVICE_SHORT_NAMES[selectedService] // Added: resolve a short name for prompts
        : null; // Added: fall back to a neutral label when no service is selected

    const options = {
        physics: { stabilization: { enabled: false } },
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
    const nodeLevelRef = useRef(0);

    const processGraphData = useCallback((proxy) => {
        const checkLinkToBR = (id, visited = new Set()) => {
            if (visited.has(id)) {
                // Cycle detected, stop recursion
                return false;
            }
            visited.add(id);

            if (RoutingGraphindexedByIpv6[id][2].length === 0 &&
                RoutingGraphindexedByIpv6[id][0] === borderrouterIpv6) {
                return true;
            } else if (RoutingGraphindexedByIpv6[id][2].length === 0 &&
                RoutingGraphindexedByIpv6[id][0] !== borderrouterIpv6) {
                return false;
            } else if (RoutingGraphindexedByIpv6[id][2] !== 0 &&
                RoutingGraphindexedByIpv6[RoutingGraphindexedByIpv6[id][2][0]] !== undefined) {
                if (checkLinkToBR(RoutingGraphindexedByIpv6[id][2][0], visited)) {
                    nodeLevelRef.current = nodeLevelRef.current + 1;
                    return true;
                }
            } else {
                return false;
            }
        };

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
            if (proxy.RoutingGraph[i][1] === false && proxy.RoutingGraph[i][2].length === 0 && i === 0) {
                nodeRole = NodeRoles.BorderRouter;
                borderrouterIpv6 = proxy.RoutingGraph[i][0];
            } else if (proxy.RoutingGraph[i][1] === true) {
                nodeRole = NodeRoles.LFN;
            } else {
                nodeRole = NodeRoles.FFN;
            }

            let parentIPv6;
            nodeLevelRef.current = 0;
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
                        level: nodeLevelRef.current,
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
                        level: nodeLevelRef.current,
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
    }, [setStateGraph, setLoading]);

    const initializeDbus = useCallback(() => {
        if (!selectedService) { // Added: do not initialize DBus until a service is selected
            if (loading) setLoading(false); // Added: hide the loading spinner when nothing is selected
            return; // Added: exit until the user picks a service
        }
        if (!serviceDbus) { // Added: ensure DBus identifiers are available before connecting
            if (loading) setLoading(false); // Added: keep the UI responsive when DBus metadata is missing
            return; // Added: exit early when DBus information is unavailable
        }
        if (active !== true) {
            if (loading) setLoading(false);
            return;
        }
        // Added: connect to the DBus endpoint for the selected service
        dbusClient.current = cockpit.dbus(
            serviceDbus.busName,
            { bus: "system" }
        );

        dbusClient.current.wait(() => {
            const proxy = dbusClient.current.proxy();
            const proxy_signal = dbusClient.current.proxy(
                "org.freedesktop.DBus.Properties",
                serviceDbus.objectPath
            ); // Added: subscribe to property updates on the selected service path
            proxy.wait().then(() => {
                if (proxy.valid === false) {
                    setHasError(true);
                    setLoading(false);
                } else if (proxy.WisunMode !== undefined) {
                    proxy_signal.addEventListener("signal", (event, name, args) => {
                        if (args[2][0] === "RoutingGraph") {
                            setTimeout(() => {
                                processGraphData(proxy);
                            }, 3000);
                        }
                    });
                    processGraphData(proxy);
                }
            });
        });
    }, [ // Added: expanded dependency list for readability
        active,
        loading,
        processGraphData,
        selectedService,
        serviceDbus
    ]); // Added: rerun initialization when the selected service changes

    useEffect(() => {
        initializeDbus();
        return () => {
            if (dbusClient.current) {
                dbusClient.current.close();
            }
        };
    }, [initializeDbus]);

    useEffect(() => {
        if (autoZoom && network !== undefined) {
            network.fit({
                animation: {
                    duration: 1000,
                    easingFunction: "linear",
                }
            });
        }
    }, [autoZoom, network]); // Removed isMounted as it was unused

    const closeDrawer = () => {
        network.unselectAll();
        setIsExpanded(false);
        setSelectedNode(null);
    };

    if (!selectedService) { // Added: prompt the user when no service is selected
        return (
            <CenteredContent> {/* Added: center the selection prompt */}
                <Alert // Added: expand props for readability
                    variant='info'
                    title="Select a service to view the network topology"
                /> {/* Added: explain why no topology is displayed */}
            </CenteredContent>
        );
    }

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
                <Alert
                    variant="info"
                    title={`Start ${selectedServiceName || 'the selected service'} to view the network topology`}
                /> {/* Added: tailor the prompt to the active service */}
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
