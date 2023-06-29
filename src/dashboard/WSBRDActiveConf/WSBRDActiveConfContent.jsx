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

import { useContext, useEffect, useState } from "react";
import {
    DescriptionList,
    DescriptionListDescription,
    DescriptionListGroup,
    DescriptionListTerm,
    Alert
} from "@patternfly/react-core";
import cockpit from 'cockpit';
import CenteredContent from "../../utils/CenteredContent";
import { AppContext } from "../../app";
import Loading from "../../utils/Loading";

const _ = cockpit.gettext;

const WSBRDActiveConfContent = () => {
    const [networkName, setNetworkName] = useState('');
    const [panID, setPanID] = useState('');
    const [size, setSize] = useState('');
    const [domain, setDomain] = useState('');

    // FAN 1.0
    const [wisunClass, setWisunClass] = useState(0);
    const [mode, setMode] = useState('');
    // FAN 1.1
    const [wisunChanPlanId, setWisunChanPlanId] = useState('');
    const [wisunPHYModeId, setWisunPHYModeId] = useState('');

    const [loading, setLoading] = useState(true);
    const [hasError, setHasError] = useState(false);

    const { active } = useContext(AppContext);

    useEffect(() => {
        // only make a dbus request if the service is active
        if (active !== true) {
            if (loading) {
                setLoading(false);
            }
            return;
        }

        const getProperties = () => {
            const dbusClient = cockpit.dbus("com.silabs.Wisun.BorderRouter", { bus: "system" });

            dbusClient.wait(() => {
                const proxy = dbusClient.proxy();

                proxy.wait(() => {
                    if (proxy.valid === false) {
                        setHasError(true);
                        setLoading(false);
                    } else if (proxy.WisunMode === undefined) {
                        // the service is not yet ready, dbus is set to be called again in one second
                        setTimeout(getProperties, 1000);
                    } else {
                        setNetworkName(proxy.data.WisunNetworkName);
                        setPanID(`0x${proxy.data.WisunPanId.toString(16).toUpperCase()}`);
                        setSize(`${proxy.data.WisunSize.toUpperCase()}`);
                        setDomain(proxy.data.WisunDomain);

                        if (proxy.data.WisunClass === 0) {
                            // for FAN 1.1
                            setWisunChanPlanId(proxy.data.WisunChanPlanId);
                            setWisunPHYModeId(proxy.data.WisunPhyModeId);
                        } else {
                            // for FAN 1.0
                            setMode(`0x${proxy.data.WisunMode.toString(16).toUpperCase()}`);
                        }

                        setWisunClass(proxy.data.WisunClass);
                        setLoading(false);
                    }
                    dbusClient.close();
                });
            });
        };

        getProperties();
    }, [active, loading]);

    if (loading) {
        return (
            <Loading />
        );
    }

    if (hasError === true || active === null) {
        return (
            <CenteredContent>
                <Alert variant='danger' title="Could not retrieve WSBRD active configuration" />
            </CenteredContent>
        );
    }

    if (active === false) {
        return (
            <CenteredContent>
                <Alert variant='info' title="Start WSBRD to view its configuration" />
            </CenteredContent>
        );
    }

    return (
        <DescriptionList isHorizontal columnModifier={{ default: '1Col', lg: '2Col' }}>
            <DescriptionListGroup>
                <DescriptionListTerm>Network Name</DescriptionListTerm>
                <DescriptionListDescription>{networkName}</DescriptionListDescription>
            </DescriptionListGroup>
            <DescriptionListGroup>
                <DescriptionListTerm>PanID</DescriptionListTerm>
                <DescriptionListDescription>{panID}</DescriptionListDescription>
            </DescriptionListGroup>
            <DescriptionListGroup>
                <DescriptionListTerm>Size</DescriptionListTerm>
                <DescriptionListDescription>{size}</DescriptionListDescription>
            </DescriptionListGroup>
            <DescriptionListGroup>
                <DescriptionListTerm>Domain</DescriptionListTerm>
                <DescriptionListDescription>{domain}</DescriptionListDescription>
            </DescriptionListGroup>
            <DescriptionListGroup>
                <DescriptionListTerm>{wisunClass === 0 ? "Channel Plan ID" : "Class"}</DescriptionListTerm>
                <DescriptionListDescription>
                    {wisunClass === 0 ? wisunChanPlanId : wisunClass}
                </DescriptionListDescription>
            </DescriptionListGroup>
            <DescriptionListGroup>
                <DescriptionListTerm>{wisunClass === 0 ? "PHY Mode ID" : "Mode"}</DescriptionListTerm>
                <DescriptionListDescription>{wisunClass === 0 ? wisunPHYModeId : mode}</DescriptionListDescription>
            </DescriptionListGroup>
        </DescriptionList>
    );
};

export default WSBRDActiveConfContent;
