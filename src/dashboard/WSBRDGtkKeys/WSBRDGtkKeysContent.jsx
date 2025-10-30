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
import Loading from "../../utils/Loading";
import { AppContext, SERVICE_SHORT_NAMES } from "../../app";
// Added: reuse service metadata for context-aware messaging
import { base64ToHex } from "../../utils/functions";

const _ = cockpit.gettext;

const WSBRDGtkKeysContent = () => {
    const [gtkKeys, setGtkKeys] = useState([]);
    const [loading, setLoading] = useState(true);
    const [hasError, setHasError] = useState(false);

    // Added: include the selected service for gating
    const { active, selectedService, serviceDbus } = useContext(AppContext);
    const selectedServiceName = selectedService
        ? SERVICE_SHORT_NAMES[selectedService] // Added: resolve a human-friendly label for prompts
        : null; // Added: fall back to a neutral label when no service is selected

    useEffect(() => {
        if (!selectedService) { // Added: avoid querying keys until a service is chosen
            return; // Added: exit early when no service selection exists
        }
        if (!serviceDbus) { // Added: ensure DBus identifiers exist before making requests
            return; // Added: exit early when DBus information is unavailable
        }

        // only make a dbus request if the service is active
        if (active !== true) {
            setLoading(false); // Added: clear loading state when the service is inactive
            return;
        }

        setLoading(true); // Added: show a spinner while fetching GTK keys
        setHasError(false); // Added: reset error state before retrieving data

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
                        setGtkKeys([...proxy.data.Gtks]);
                        setLoading(false);
                    }
                    dbusClient.close();
                });
            });
        };

        getProperties();
    }, [active, selectedService, serviceDbus]); // Added: refresh GTK data when service or selection changes

    if (!selectedService) { // Added: prompt the user to choose a service before displaying GTK information
        return (
            <CenteredContent> {/* Added: center the call-to-action message */}
                <Alert // Added: expand props for readability
                    variant='info'
                    title="Select a service to view GTK keys"
                /> {/* Added: explain why no data is shown */}
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
                <Alert variant='danger' title="Could not retrieve GTK Keys" />
            </CenteredContent>
        );
    }

    if (active === false) {
        return (
            <CenteredContent>
                <Alert
                    variant='info'
                    title={`Start ${selectedServiceName || 'the selected service'} to view its GTK Keys`}
                /> {/* Added: tailor the prompt to the current service */}
            </CenteredContent>
        );
    }

    return (
        <DescriptionList isHorizontal>
            {
                gtkKeys.map((gtkKey, idx) => {
                    return (
                        <DescriptionListGroup key={idx}>
                            <DescriptionListTerm>GTK [{idx}]</DescriptionListTerm>
                            <DescriptionListDescription>{base64ToHex(gtkKey)}</DescriptionListDescription>
                        </DescriptionListGroup>
                    );
                })
            }
        </DescriptionList>
    );
};

export default WSBRDGtkKeysContent;
