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
import { Alert, TextArea } from "@patternfly/react-core";
import cockpit from 'cockpit';
import CenteredContent from "../../utils/CenteredContent";
import Loading from "../../utils/Loading";
import { AppContext } from "../../app";

const _ = cockpit.gettext;

const WSBRDConfEditorContent = () => {
    const file = cockpit.file("/etc/wsbrd.conf", { superuser: "try" });

    const [content, setContent] = useState(undefined);
    const [cockpitTag, setCockpitTag] = useState(undefined);
    const [hasError, setHasError] = useState(false);
    const [error, setError] = useState(null);
    const { socAgentActive } = useContext(AppContext);

    useEffect(() => {
        file.read().then((data, tag) => {
            // if data is null it means the file does not exist
            if (data === null) {
                setHasError(true);
                setError('File does not exist, check your WSBRD configuration');
                setContent(null);
            } else {
                setHasError(false);
                setError(null);
                setContent(data);
                setCockpitTag(tag);
            }
        })
            .catch((err) => {
                console.log(err);
                setHasError(true);
                setError('Could not read configuration file');
            });
    }, [file]);

    const onContentChange = (value) => {
        setContent(value);
        file.replace(value, cockpitTag).then((tag) => setCockpitTag(tag))
            .catch((err) => {
                console.log(err);
                setHasError(true);
                setError('Unable to save changes, please refresh the page');
            });
    };

    if (content === undefined) {
        return (
            <Loading />
        );
    }

    if (hasError) {
        return (
            <CenteredContent>
                <Alert variant='danger' title={error} />
            </CenteredContent>
        );
    }

    return (
        <>
            {socAgentActive && (
                <Alert
                    variant='info'
                    isInline
                    title="Wi-SUN SoC Border Router Agent service is active. Editing is disabled."
                />
            )}
            <TextArea
                style={{ height: '100%' }}
                aria-label="wsbrd-conf"
                value={content}
                onChange={onContentChange}
                isDisabled={socAgentActive}
                resizeOrientation='vertical'

            />
        </>
    );
};

export default WSBRDConfEditorContent;
