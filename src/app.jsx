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
    setLoading: undefined
});

const App = () => {
    const [active, setActive] = useState(undefined);
    const [loading, setLoading] = useState(true);
    const [activeTab, setActiveTab] = useState(0);

    const dashboardRef = createRef();
    const topologyRef = createRef();

    useEffect(() => {
        cockpit.spawn(["systemctl", "show", "-p", "ActiveState", "--value", "wisun-borderrouter.service"],
            { superuser: "require" })
            .then((data) => {
                if (data.trim().localeCompare('active') === 0) {
                    setActive(true);
                } else if (data.trim().localeCompare('inactive') === 0) {
                    setActive(false);
                } else {
                    setActive(null);
                }

                setLoading(false);
            })
            .catch((err) => {
                console.log(err);
                setActive(null);
                setLoading(false);
            });
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
            <PageSection isWidthLimited>
                {
                    (active === undefined || loading)
                        ? <Loading />
                        : (
                            <AppContext.Provider value={{ active, setActive, loading, setLoading }}>
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
