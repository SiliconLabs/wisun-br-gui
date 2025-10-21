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

export const AppContext = createContext({
    active: undefined,
    setActive: undefined,
    loading: undefined,
    setLoading: undefined,
    wsbrdInstalled: undefined,
    setWsbrdInstalled: undefined,
    socAgentActive: undefined,
    setSocAgentActive: undefined
});

const App = () => {
    const [active, setActive] = useState(undefined);
    const [loading, setLoading] = useState(true);
    const [activeTab, setActiveTab] = useState(0);
    const [wsbrdInstalled, setWsbrdInstalled] = useState(undefined);
    const [socAgentActive, setSocAgentActive] = useState(false);

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

        const wsbrdPromise = cockpit.spawn([
            "systemctl",
            "show",
            "-p",
            "LoadState",
            "-p",
            "ActiveState",
            "--value",
            "wisun-borderrouter.service"
        ], { superuser: "require" })
            .then((data) => {
                if (!isMounted) {
                    return;
                }

                const { loadState, activeState } = parseServiceStates(data);
                const isInstalled = loadState !== null && loadState.localeCompare('not-found') !== 0;

                setWsbrdInstalled(isInstalled);

                if (!isInstalled) {
                    setActive(null);
                    return;
                }

                if (activeState && activeState.localeCompare('active') === 0) {
                    setActive(true);
                } else if (activeState && activeState.localeCompare('inactive') === 0) {
                    setActive(false);
                } else {
                    setActive(null);
                }
            })
            .catch((err) => {
                console.log(err);
                if (!isMounted) {
                    return;
                }

                setWsbrdInstalled(false);
                setActive(null);
            });

        const socAgentPromise = cockpit.spawn([
            "systemctl",
            "show",
            "-p",
            "LoadState",
            "-p",
            "ActiveState",
            "--value",
            "wisun-soc-br-agent.service"
        ], { superuser: "require" })
            .then((data) => {
                if (!isMounted) {
                    return;
                }

                const { loadState, activeState } = parseServiceStates(data);
                const isInstalled = loadState !== null && loadState.localeCompare('not-found') !== 0;

                setSocAgentActive(isInstalled && activeState && activeState.localeCompare('active') === 0);
            })
            .catch((err) => {
                console.log(err);
                if (!isMounted) {
                    return;
                }

                setSocAgentActive(false);
            });

        Promise.allSettled([wsbrdPromise, socAgentPromise]).then(() => {
            if (isMounted) {
                setLoading(false);
            }
        });

        return () => {
            isMounted = false;
        };
    }, [active]);

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
                    (active === undefined || loading)
                        ? <Loading />
                        : (
                            <AppContext.Provider
                                value={{
                                    active,
                                    setActive,
                                    loading,
                                    setLoading,
                                    wsbrdInstalled,
                                    setWsbrdInstalled,
                                    socAgentActive,
                                    setSocAgentActive
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
