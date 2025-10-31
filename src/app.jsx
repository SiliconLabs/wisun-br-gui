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
import { useState, useEffect, createContext, createRef } from 'react';
import cockpit from 'cockpit';
import Dashboard from './dashboard/Dashboard';
import Topology from './topology/Topology';
import Loading from './utils/Loading';

const _ = cockpit.gettext;

export const SERVICE_UNITS = {
    linux: 'wisun-borderrouter.service',
    soc: 'wisun-soc-br-agent.service'
};

export const SERVICE_DBUS = {
    linux: {
        busName: 'com.silabs.Wisun.BorderRouter',
        objectPath: '/com/silabs/Wisun/BorderRouter'
    },
    soc: {
        busName: 'com.silabs.Wisun.SocBorderRouterAgent',
        objectPath: '/com/silabs/Wisun/SocBorderRouterAgent'
    }
};

export const SERVICE_LABELS = {
    linux: _('Linux Border Router Service'),
    soc: _('SoC Border Router Agent Service')
};

export const SERVICE_SHORT_NAMES = {
    linux: _('WSBRD'),
    soc: _('SoC Border Router Agent')
};

/**
 * AppContext exposes service lifecycle data so that dashboard and topology
 * views can stay in sync while reacting to the same selection.
 */
export const AppContext = createContext({
    active: undefined,
    loading: undefined,
    setLoading: undefined,
    services: undefined,
    selectedService: undefined,
    setSelectedService: undefined,
    refreshServices: undefined,
    serviceDbus: undefined
});

const initialServiceState = {
    installed: undefined,
    active: undefined,
    loadState: null,
    activeState: null
};

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

const computeActiveState = (activeState) => {
    if (!activeState) {
        return null;
    }
    if (activeState.localeCompare('active') === 0) {
        return true;
    }
    if (activeState.localeCompare('inactive') === 0) {
        return false;
    }
    return null;
};

/**
 * The application orchestrates service discovery, shares the result via
 * context, and switches between the dashboard and topology modules.
 */
const App = () => {
    const [loading, setLoading] = useState(true);
    const [activeTab, setActiveTab] = useState(0);
    const [services, setServices] = useState({
        linux: { ...initialServiceState },
        soc: { ...initialServiceState }
    });
    const [selectedService, setSelectedService] = useState(undefined);
    const [refreshCounter, setRefreshCounter] = useState(0);

    const dashboardRef = createRef();
    const topologyRef = createRef();

    useEffect(() => {
        let isMounted = true;

        setLoading(true);

        const nextServices = {
            linux: { ...initialServiceState, installed: false },
            soc: { ...initialServiceState, installed: false }
        };

        const servicePromises = Object.entries(SERVICE_UNITS).map(([key, unit]) => {
            return cockpit.spawn([
                'systemctl',
                'show',
                '-p',
                'LoadState',
                '-p',
                'ActiveState',
                '--value',
                unit
            ], { superuser: 'require' })
                .then((data) => {
                    if (!isMounted) {
                        return;
                    }

                    const { loadState, activeState } = parseServiceStates(data);
                    const isInstalled = loadState !== null && loadState.localeCompare('not-found') !== 0;

                    nextServices[key] = {
                        installed: isInstalled,
                        active: isInstalled ? computeActiveState(activeState) : null,
                        loadState,
                        activeState
                    };
                })
                .catch((err) => {
                    console.log(err);
                    if (!isMounted) {
                        return;
                    }

                    nextServices[key] = {
                        installed: false,
                        active: null,
                        loadState: null,
                        activeState: null
                    };
                });
        });

        Promise.allSettled(servicePromises).then(() => {
            if (!isMounted) {
                return;
            }

            setServices(nextServices);
            setLoading(false);
            setSelectedService((prevSelected) => {
                if (prevSelected && nextServices[prevSelected]?.installed) {
                    return prevSelected;
                }
                const installedServices = Object.entries(nextServices)
                    .filter(([, service]) => service.installed);
                if (installedServices.length === 1) {
                    return installedServices[0][0];
                }
                return undefined;
            });
        });

        return () => {
            isMounted = false;
        };
    }, [refreshCounter]);

    const active = selectedService ? services[selectedService]?.active : undefined;
    const serviceDbus = selectedService
        ? SERVICE_DBUS[selectedService]
        : null;
    const refreshServices = () => setRefreshCounter((value) => value + 1);

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
                        : (
                            <AppContext.Provider
                                value={{
                                    active,
                                    loading,
                                    setLoading,
                                    services,
                                    selectedService,
                                    setSelectedService,
                                    refreshServices,
                                    serviceDbus
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
