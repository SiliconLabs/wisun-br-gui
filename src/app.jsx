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
    Page,
    PageSection,
    PageSectionVariants,
    TextContent,
    Text,
    Flex,
    FlexItem,
    Tabs,
    Tab,
    TabTitleText
} from '@patternfly/react-core';
import { useState, useEffect, createContext, createRef } from 'react'; // Added: import React hooks used below
import cockpit from 'cockpit';
import Dashboard from './dashboard/Dashboard';
import Topology from './topology/Topology';
import Loading from './utils/Loading';

const _ = cockpit.gettext;

export const SERVICE_UNITS = { // Added: expose service names for reuse across the app
    linux: 'wisun-borderrouter.service', // Added: systemd unit name for the Linux border router
    soc: 'wisun-soc-br-agent.service' // Added: systemd unit name for the SoC border router agent
};

export const SERVICE_DBUS = { // Added: map each service to its DBus identifiers
    linux: { // Added: identifiers for the Linux border router service
        busName: 'com.silabs.Wisun.BorderRouter', // Added: DBus bus name exposed by wsbrd
        objectPath: '/com/silabs/Wisun/BorderRouter' // Added: DBus object path used for property monitoring
    },
    soc: { // Added: identifiers for the SoC border router agent service
        busName: 'com.silabs.Wisun.SocBorderRouterAgent', // Added: DBus bus name exposed by the SoC agent
        objectPath: '/com/silabs/Wisun/SocBorderRouterAgent' // Added: DBus object path exposed by the SoC agent
    }
};

export const SERVICE_LABELS = { // Added: user-facing labels for each service
    linux: _('Linux Border Router Service'), // Added: label for the Linux border router service
    soc: _('SoC Border Router Agent Service') // Added: label for the SoC border router agent service
};

export const SERVICE_SHORT_NAMES = { // Added: short names used in inline messages
    linux: _('WSBRD'), // Added: shorthand for the Linux border router service
    soc: _('SoC Border Router Agent') // Added: shorthand for the SoC border router agent service
};

export const AppContext = createContext({
    active: undefined, // Added: tracks the active state of the selected service
    loading: undefined, // Added: exposes loading status to child components
    setLoading: undefined, // Added: allows children to toggle the global loading indicator
    services: undefined, // Added: shares aggregated service metadata with descendants
    selectedService: undefined, // Added: stores which service the user picked
    setSelectedService: undefined, // Added: lets children update the selected service
    refreshServices: undefined, // Added: function to trigger service status refreshes
    serviceDbus: undefined // Added: exposes DBus identifiers for the active service
});

const App = () => {
    const [loading, setLoading] = useState(true); // Added: manage the global loading indicator
    const [activeTab, setActiveTab] = useState(0);
    // Added: track metadata for both services
    const [services, setServices] = useState({
        // Added: seed Linux metadata
        linux: {
            installed: undefined,
            active: undefined,
            loadState: null,
            activeState: null
        },
        // Added: seed SoC metadata
        soc: {
            installed: undefined,
            active: undefined,
            loadState: null,
            activeState: null
        }
    });
    const [selectedService, setSelectedService] = useState(undefined); // Added: store the chosen service
    const [refreshCounter, setRefreshCounter] = useState(0); // Added: trigger status refreshes

    const dashboardRef = createRef();
    const topologyRef = createRef();

    useEffect(() => {
        let isMounted = true;

        setLoading(true);

        const parseServiceStates = (data) => {
            const trimmedData = data.trim();
            if (trimmedData.length === 0) {
                return { loadState: null, activeState: null };
            }

            const values = trimmedData.split('\n');

            return {
                loadState: values[0] || null,
                activeState: values[1] || null
            };
        };
        const computeActiveState = (activeState) => { // Added: normalize systemd state values
            if (!activeState) { // Added: guard against missing state strings
                return null; // Added: treat missing information as unknown
            }
            if (activeState.localeCompare('active') === 0) { // Added: map "active" to true
                return true; // Added: reflect active state as true
            }
            if (activeState.localeCompare('inactive') === 0) { // Added: map "inactive" to false
                return false; // Added: reflect inactive state as false
            }
            return null; // Added: default to unknown for other states
        };

        const nextServices = { // Added: collect refreshed service data
            linux: {
                installed: false,
                active: undefined,
                loadState: null,
                activeState: null
            }, // Added: initialize Linux snapshot
            soc: {
                installed: false,
                active: undefined,
                loadState: null,
                activeState: null
            } // Added: initialize SoC snapshot
        };

        // Added: query each service status
        const servicePromises = Object.entries(SERVICE_UNITS).map(([key, unit]) => {
            return cockpit.spawn([ // Added: invoke systemctl show for the current service
                'systemctl', // Added: call systemctl to query unit properties
                'show', // Added: request unit details
                '-p', // Added: specify property retrieval
                'LoadState', // Added: ask for the load state property
                '-p', // Added: continue listing properties
                'ActiveState', // Added: ask for the active state property
                '--value', // Added: request only the property values
                unit // Added: target the current systemd unit
            ], { superuser: 'require' })
                .then((data) => { // Added: process successful service status queries
                    if (!isMounted) { // Added: prevent state updates when component is unmounted
                        return; // Added: exit early if component is not mounted
                    }

                    // Added: parse systemctl output for the service
                    const { loadState, activeState } = parseServiceStates(data);
                    // Added: detect installation
                    const isInstalled = loadState !== null && loadState.localeCompare('not-found') !== 0;

                    nextServices[key] = { // Added: store refreshed service information
                        installed: isInstalled, // Added: capture installation status
                        // Added: compute active status when installed
                        active: isInstalled ? computeActiveState(activeState) : null,
                        loadState, // Added: keep the raw load state for reference
                        activeState // Added: keep the raw active state for reference
                    };
                })
                .catch((err) => { // Added: handle systemctl errors per service
                    console.log(err); // Added: log errors for troubleshooting
                    if (!isMounted) { // Added: avoid state writes when unmounted
                        return; // Added: exit if component is not mounted
                    }

                    nextServices[key] = { // Added: default to unavailable service metadata on errors
                        installed: false, // Added: mark service as unavailable on failure
                        active: null, // Added: mark active state as unknown on failure
                        loadState: null, // Added: clear load state after failure
                        activeState: null // Added: clear active state after failure
                    };
                });
        });

        Promise.allSettled(servicePromises).then(() => { // Added: wait for both service queries to finish
            if (!isMounted) { // Added: verify component is still mounted before updating state
                return; // Added: exit early when unmounted
            }

            setServices(nextServices); // Added: commit refreshed service metadata to state
            setLoading(false); // Added: hide the global loading indicator after refreshing
            setSelectedService((prevSelected) => { // Added: adjust selected service when installation states change
                // Added: preserve previous selection if still valid
                if (prevSelected && nextServices[prevSelected]?.installed) {
                    return prevSelected; // Added: keep user choice when possible
                }
                const installedServices = Object.entries(nextServices)
                    .filter(([, service]) => service.installed); // Added: gather installed services
                // Added: auto-select single available service
                if (installedServices.length === 1) {
                    // Added: pick the only installed service automatically
                    return installedServices[0][0];
                }
                return undefined; // Added: require manual selection when zero or multiple services are installed
            });
        });

        return () => {
            isMounted = false;
        };
    }, [refreshCounter]); // Added: refresh service states whenever a refresh is requested

    // Added: derive the active state of the chosen service
    const active = selectedService ? services[selectedService]?.active : undefined;
    const serviceDbus = selectedService
        ? SERVICE_DBUS[selectedService] // Added: resolve DBus identifiers when a service is chosen
        : null; // Added: fall back to null when no service is selected
    const refreshServices = () => setRefreshCounter((value) => value + 1); // Added: helper to trigger a status refresh

    return (
        <Page
            style={{ height: '100vh' }} additionalGroupedContent={
                <PageSection variant={PageSectionVariants.light} isWidthLimited>
                    <Flex direction={{ default: 'column' }}>
                        <FlexItem>
                            <TextContent>
                                <Text component="h1">Wi-SUN Border Router</Text>
                            </TextContent>
                        </FlexItem>
                        <FlexItem>
                            <Tabs activeKey={activeTab} onSelect={(e, k) => setActiveTab(k)} hasBorderBottom={false}>
                                <Tab
                                    eventKey={0}
                                    title={<TabTitleText>Dashboard</TabTitleText>}
                                    tabContentId='dashboard'
                                    tabContentRef={dashboardRef}
                                />
                                <Tab
                                    eventKey={1}
                                    title={<TabTitleText>Topology</TabTitleText>}
                                    tabContentId='topology'
                                    tabContentRef={topologyRef}
                                />
                            </Tabs>
                        </FlexItem>
                    </Flex>
                </PageSection>
            }
        >
            <PageSection>
                {
                    loading
                        ? <Loading />
                        : ( // Added: render application content even when no service is selected
                            <AppContext.Provider
                                value={{
                                    active,
                                    loading,
                                    setLoading,
                                    services, // Added: share both service states with the rest of the app
                                    selectedService, // Added: provide the chosen service to children
                                    setSelectedService, // Added: let children update the selected service
                                    refreshServices, // Added: allow components to refresh service states after actions
                                    serviceDbus // Added: provide DBus information for the selected service
                                }}
                            >
                                {
                                    activeTab === 0 && <Dashboard />
                                }
                                {
                                    activeTab === 1 && <Topology />
                                }
                            </AppContext.Provider>
                        )
                }
            </PageSection>
        </Page>
    );
};

export default App;
