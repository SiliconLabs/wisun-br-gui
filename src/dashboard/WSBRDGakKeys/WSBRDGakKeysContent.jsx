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
import { AppContext, SERVICE_SHORT_NAMES } from "../../app";
import Loading from "../../utils/Loading";
import { base64ToHex } from "../../utils/functions";

const _ = cockpit.gettext;

/**
 * Displays GAK keys reported by the active service via DBus.
 */
const WSBRDGakKeysContent = () => {
    const [gakKeys, setGakKeys] = useState([]);
    const [loading, setLoading] = useState(true);
    const [hasError, setHasError] = useState(false);

    const { active, selectedService, serviceDbus } = useContext(AppContext);
    const selectedServiceName = selectedService
        ? SERVICE_SHORT_NAMES[selectedService]
        : null;

    useEffect(() => {
        if (!selectedService || !serviceDbus) {
            return;
        }

        if (active !== true) {
            setLoading(false);
            return;
        }

        setLoading(true);
        setHasError(false);

        const getProperties = () => {
            const dbusClient = cockpit.dbus(
                serviceDbus.busName,
                { bus: "system" }
            );

            dbusClient.wait(() => {
                const proxy = dbusClient.proxy();

                proxy.wait().then(() => {
                    if (proxy.valid === false) {
                        setHasError(true);
                        setLoading(false);
                    } else if (proxy.WisunMode === undefined) {
                        setTimeout(getProperties, 1000);
                    } else {
                        setGakKeys([...proxy.data.Gaks]);
                        setLoading(false);
                    }
                    dbusClient.close();
                });
            });
        };

        getProperties();
    }, [active, selectedService, serviceDbus]);

    if (!selectedService) {
        return (
            <CenteredContent>
                <Alert
                    variant='info'
                    title="Select a service to view GAK keys"
                />
            </CenteredContent>
        );
    }

    if (loading) {
        return (
            <Loading />
        );
    }

    if (hasError === true || active === null) {
        return (
            <CenteredContent>
                <Alert variant='danger' title="Could not retrieve GAK Keys" />
            </CenteredContent>
        );
    }

    if (active === false) {
        return (
            <CenteredContent>
                <Alert
                    variant='info'
                    title={`Start ${selectedServiceName || 'the selected service'} to view its GAK Keys`}
                />
            </CenteredContent>
        );
    }

    return (
        <DescriptionList isHorizontal>
            {
                gakKeys.map((gakKey, idx) => {
                    return (
                        <DescriptionListGroup key={idx}>
                            <DescriptionListTerm>GAK [{idx}]</DescriptionListTerm>
                            <DescriptionListDescription>{base64ToHex(gakKey)}</DescriptionListDescription>
                        </DescriptionListGroup>
                    );
                })
            }
        </DescriptionList>
    );
};

export default WSBRDGakKeysContent;
