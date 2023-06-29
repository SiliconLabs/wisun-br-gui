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
import { AppContext } from "../../app";
import { base64ToHex } from "../../utils/functions";

const _ = cockpit.gettext;

const WSBRDGtkKeysContent = () => {
    const [gtkKeys, setGtkKeys] = useState([]);
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
    }, [active, loading]);

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
                <Alert variant='info' title="Start WSBRD to view its GTK Keys" />
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
