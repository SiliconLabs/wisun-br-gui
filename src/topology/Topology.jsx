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
    useRef,
    useMemo
} from "react";
import cockpit from "cockpit";
import { AppContext, SERVICE_SHORT_NAMES } from "../app";
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
const EDGE_COLOR = "#0f62fe";

const GRAPH_OPTIONS = {
    physics: { stabilization: { enabled: false } },
    layout: {
        improvedLayout: true,
        hierarchical: {
            enabled: true,
            direction: 'UD',
            sortMethod: 'hubsize',
            blockShifting: false,
            edgeMinimization: true,
            parentCentralization: true,
        },
    },
    edges: {
        color: EDGE_COLOR,
        arrows: { to: false }
    },
    height: '100%',
    width: '100%'
};

/**
 * Topology renders the Wi-SUN routing graph for the active service and keeps
 * the visualization synchronized with DBus updates.
 */
const Topology = () => {
    const dbusClient = useRef(null);
    const [loading, setLoading] = useState(true);
    const [stateGraph, setStateGraph] = useState({
        nodes: [],
        edges: []
    });
    const [hasError, setHasError] = useState(false);
    const [isExpanded, setIsExpanded] = useState(false);
    const [network, setNetwork] = useState();
    const [selectedNode, setSelectedNode] = useState(null);
    const [autoZoom, setAutoZoom] = useState(true);

    const { active, selectedService, serviceDbus } = useContext(AppContext);
    const selectedServiceName = selectedService
        ? SERVICE_SHORT_NAMES[selectedService]
        : null;

    const nodeLevelRef = useRef(0);
    const animationFrameRef = useRef(null);
    const proxySignalRef = useRef(null);
    const proxySignalHandlerRef = useRef(null);

    const cancelScheduledUpdate = () => {
        if (!animationFrameRef.current) {
            return;
        }

        const { id, type } = animationFrameRef.current;
        if (type === 'raf' && typeof window !== 'undefined' && typeof window.cancelAnimationFrame === 'function') {
            window.cancelAnimationFrame(id);
        } else {
            clearTimeout(id);
        }
        animationFrameRef.current = null;
    };

    const processGraphData = useCallback((proxy) => {
        if (!proxy || !Array.isArray(proxy.RoutingGraph)) {
            setStateGraph({ nodes: [], edges: [] });
            setLoading(false);
            return;
        }

        const routingGraphByIpv6 = {};
        let borderRouterIpv6;

        proxy.RoutingGraph.forEach((entry) => {
            routingGraphByIpv6[entry[0]] = entry;
            if (entry[1] === false && entry[2].length === 0) {
                borderRouterIpv6 = entry[0];
            }
        });

        const checkLinkToBorderRouter = (id, visited = new Set()) => {
            if (visited.has(id)) {
                return false;
            }
            visited.add(id);

            const entry = routingGraphByIpv6[id];
            if (!entry) {
                return false;
            }

            if (entry[2].length === 0) {
                return entry[0] === borderRouterIpv6;
            }

            const parentId = entry[2][0];
            if (parentId !== undefined && routingGraphByIpv6[parentId] !== undefined) {
                if (checkLinkToBorderRouter(parentId, visited)) {
                    nodeLevelRef.current += 1;
                    return true;
                }
            }

            return false;
        };

        const nodes = [];
        const edges = [];

        for (let i = 0; i < proxy.RoutingGraph.length; i += 1) {
            if (proxy.RoutingGraph[i][1] === undefined) {
                continue;
            }

            const ipv6 = base64ToHex(proxy.RoutingGraph[i][0]);

            let nodeRole;
            if (proxy.RoutingGraph[i][1] === false && proxy.RoutingGraph[i][2].length === 0) {
                nodeRole = NodeRoles.BorderRouter;
            } else if (proxy.RoutingGraph[i][1] === true) {
                nodeRole = NodeRoles.LFN;
            } else {
                nodeRole = NodeRoles.FFN;
            }

            let parentIPv6;
            nodeLevelRef.current = 0;
            if (checkLinkToBorderRouter(proxy.RoutingGraph[i][0])) {
                if (proxy.RoutingGraph[i][2][0] !== undefined) {
                    parentIPv6 = base64ToHex(proxy.RoutingGraph[i][2][0]);
                }
                nodes.push({
                    id: ipv6,
                    label: ipv6.slice(ipv6.length - 4),
                    color: NodeRolesColors[nodeRole],
                    ipv6: beautifyIpv6String(ipv6),
                    parentIPv6: parentIPv6 ? beautifyIpv6String(parentIPv6) : undefined,
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

        setStateGraph({
            nodes,
            edges
        });

        setLoading(false);
    }, []);

    const initializeDbus = useCallback(() => {
        if (!selectedService || !serviceDbus || active !== true) {
            cancelScheduledUpdate();
            setLoading((prev) => (prev ? false : prev));
            setStateGraph((prev) => (prev.nodes.length || prev.edges.length ? { nodes: [], edges: [] } : prev));
            return;
        }

        setLoading(true);
        setHasError(false);

        cancelScheduledUpdate();

        if (proxySignalRef.current && proxySignalHandlerRef.current) {
            proxySignalRef.current.removeEventListener("signal", proxySignalHandlerRef.current);
            proxySignalHandlerRef.current = null;
            proxySignalRef.current = null;
        }

        dbusClient.current = cockpit.dbus(
            serviceDbus.busName,
            { bus: "system" }
        );

        dbusClient.current.wait(() => {
            if (!dbusClient.current) {
                return;
            }

            const proxy = dbusClient.current.proxy();
            if (!proxy) {
                return;
            }

            const proxySignal = dbusClient.current.proxy(
                "org.freedesktop.DBus.Properties",
                serviceDbus.objectPath
            );

            proxy.wait().then(() => {
                if (proxy.valid === false) {
                    setHasError(true);
                    setLoading(false);
                } else if (proxy.WisunMode !== undefined) {
                    const handleSignal = (event, name, args) => {
                        if (args && args[2] && args[2][0] === "RoutingGraph") {
                            cancelScheduledUpdate();
                            if (typeof window !== 'undefined' && typeof window.requestAnimationFrame === 'function') {
                                const id = window.requestAnimationFrame(() => {
                                    animationFrameRef.current = null;
                                    processGraphData(proxy);
                                });
                                animationFrameRef.current = { id, type: 'raf' };
                            } else {
                                const id = setTimeout(() => {
                                    animationFrameRef.current = null;
                                    processGraphData(proxy);
                                }, 0);
                                animationFrameRef.current = { id, type: 'timeout' };
                            }
                        }
                    };
                    if (proxySignal && typeof proxySignal.addEventListener === 'function') {
                        proxySignal.addEventListener("signal", handleSignal);
                        proxySignalRef.current = proxySignal;
                        proxySignalHandlerRef.current = handleSignal;
                    } else {
                        proxySignalRef.current = null;
                        proxySignalHandlerRef.current = null;
                    }
                    processGraphData(proxy);
                }
            });
        });
    }, [
        active,
        processGraphData,
        selectedService,
        serviceDbus
    ]);

    useEffect(() => {
        initializeDbus();
        return () => {
            cancelScheduledUpdate();
            if (proxySignalRef.current && proxySignalHandlerRef.current) {
                proxySignalRef.current.removeEventListener("signal", proxySignalHandlerRef.current);
                proxySignalHandlerRef.current = null;
                proxySignalRef.current = null;
            }
            if (dbusClient.current) {
                dbusClient.current.close();
                dbusClient.current = null;
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
    }, [autoZoom, network]);

    const handleSelectNode = useCallback((event) => {
        const { nodes } = event;
        if (!nodes || nodes.length === 0) {
            return;
        }

        const graphNode = stateGraph.nodes.find((n) => n.id.localeCompare(nodes[0]) === 0);

        if (graphNode !== undefined) {
            setIsExpanded(true);
            setSelectedNode(graphNode);
        }
    }, [stateGraph.nodes]);

    const handleDeselectNode = useCallback(() => {
        setIsExpanded(false);
        setSelectedNode(null);
    }, []);

    const handleDisableAutoZoom = useCallback(() => {
        setAutoZoom(false);
    }, []);

    const events = useMemo(() => ({
        selectNode: handleSelectNode,
        deselectNode: handleDeselectNode,
        dragStart: handleDisableAutoZoom,
        zoom: handleDisableAutoZoom
    }), [handleSelectNode, handleDeselectNode, handleDisableAutoZoom]);

    const handleGetNetwork = useCallback((n) => {
        setNetwork(n);
    }, []);

    const closeDrawer = () => {
        if (network) {
            network.unselectAll();
        }
        setIsExpanded(false);
        setSelectedNode(null);
    };

    if (!selectedService) {
        return (
            <CenteredContent>
                <Alert
                    variant='info'
                    title="Select a service to view the network topology"
                />
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
                />
            </CenteredContent>
        );
    }

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
                        graph={stateGraph} options={GRAPH_OPTIONS} events={events} getNetwork={handleGetNetwork}
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
