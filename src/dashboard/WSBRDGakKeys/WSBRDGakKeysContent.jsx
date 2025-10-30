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
import { AppContext, SERVICE_SHORT_NAMES } from "../../app"; // Added: use shared service labels for dynamic messaging
import Loading from "../../utils/Loading";
import { base64ToHex } from "../../utils/functions";

const _ = cockpit.gettext;

const WSBRDGakKeysContent = () => {
    const [gakKeys, setGakKeys] = useState([]);
    const [loading, setLoading] = useState(true);
    const [hasError, setHasError] = useState(false);

    // Added: include the selected service for gating
    const { active, selectedService, serviceDbus } = useContext(AppContext);
    const selectedServiceName = selectedService
        ? SERVICE_SHORT_NAMES[selectedService] // Added: resolve a friendly service name for prompts
        : null; // Added: fall back to a neutral label when no service is selected

    useEffect(() => {
        if (!selectedService) { // Added: avoid querying keys until a service is chosen
            return; // Added: bail out when no service is selected
        }
        if (!serviceDbus) { // Added: ensure DBus identifiers are available before querying
            return; // Added: exit early when DBus information is missing
        }

        // only make a dbus request if the service is active
        if (active !== true) {
            setLoading(false); // Added: clear loading when the service is not running
            return;
        }

        setLoading(true); // Added: indicate loading before fetching GAK keys
        setHasError(false); // Added: reset error state before issuing a fresh request

        const getProperties = () => {
            const dbusClient = cockpit.dbus( // Added: target the selected service DBus endpoint
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
                        // the service is not yet ready, dbus is set to be called again in one second
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
    }, [active, selectedService, serviceDbus]); // Added: refresh GAK data when selection or status changes

    if (!selectedService) { // Added: prompt the user to pick a service before listing GAK keys
        return (
            <CenteredContent> {/* Added: center the call-to-action message */}
                <Alert // Added: expand props for readability
                    variant='info'
                    title="Select a service to view GAK keys"
                /> {/* Added: explain why the list is empty */}
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
                /> {/* Added: tailor the prompt to the active service */}
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
