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
    useCallback,
    useContext,
    useEffect,
    useMemo,
    useRef,
    useState,
} from "react";
import { Alert, TextArea } from "@patternfly/react-core";
import cockpit from 'cockpit';
import CenteredContent from "../../utils/CenteredContent";
import Loading from "../../utils/Loading";
import { AppContext } from "../../app";

const _ = cockpit.gettext;

const WSBRDConfEditorContent = () => {
    const file = useMemo(
        () => cockpit.file("/etc/wsbrd.conf", { superuser: "try" }),
        []
    );

    const [content, setContent] = useState(undefined);
    const cockpitTagRef = useRef(undefined);
    const [loadError, setLoadError] = useState(null);
    const [saveError, setSaveError] = useState(null);
    const [isLoading, setIsLoading] = useState(true);
    const saveTimeoutRef = useRef(null);
    const latestValueRef = useRef(null);
    const saveQueueRef = useRef(Promise.resolve());
    const { selectedService } = useContext(AppContext);

    useEffect(() => {
        let isCancelled = false;

        if (selectedService !== 'linux') {
            setIsLoading(false);
            setLoadError(null);
            return () => {
                isCancelled = true;
            };
        }

        setIsLoading(true);
        setLoadError(null);

        file.read()
            .then((data, tag) => {
                if (isCancelled) {
                    return;
                }

                if (data === null) {
                    setLoadError('File does not exist, check your WSBRD configuration');
                    setContent('');
                    cockpitTagRef.current = undefined;
                } else {
                    setContent(data);
                    cockpitTagRef.current = tag;
                    latestValueRef.current = data;
                }

                setIsLoading(false);
            })
            .catch((err) => {
                if (isCancelled) {
                    return;
                }

                console.log(err);
                setLoadError('Could not read configuration file');
                setIsLoading(false);
            });

        return () => {
            isCancelled = true;
        };
    }, [file, selectedService]);

    const flushSaveQueue = useCallback(() => {
        if (saveTimeoutRef.current) {
            clearTimeout(saveTimeoutRef.current);
            saveTimeoutRef.current = null;
        }

        if (latestValueRef.current === null || cockpitTagRef.current === undefined) {
            return saveQueueRef.current;
        }

        saveQueueRef.current = saveQueueRef.current
            .then(() => file.replace(latestValueRef.current, cockpitTagRef.current))
            .then((tag) => {
                cockpitTagRef.current = tag;
                setSaveError(null);
            })
            .catch((err) => {
                console.log(err);
                setSaveError('Unable to save changes, please refresh the page');
            });

        return saveQueueRef.current;
    }, [file]);

    const scheduleSave = useCallback(() => {
        if (saveTimeoutRef.current) {
            clearTimeout(saveTimeoutRef.current);
        }

        saveTimeoutRef.current = setTimeout(() => {
            flushSaveQueue();
        }, 400);
    }, [flushSaveQueue]);

    const onContentChange = (value) => {
        setContent(value);
        latestValueRef.current = value;
        setSaveError(null);
        scheduleSave();
    };

    useEffect(() => () => {
        if (saveTimeoutRef.current) {
            clearTimeout(saveTimeoutRef.current);
        }
    }, []);

    if (selectedService !== 'linux') {
        return (
            <CenteredContent>
                <Alert
                    variant='info'
                    title="Select the Linux Border Router Service to edit wsbrd.conf"
                />
            </CenteredContent>
        );
    }

    if (isLoading || content === undefined) {
        return (
            <Loading />
        );
    }

    if (loadError) {
        return (
            <CenteredContent>
                <Alert variant='danger' title={loadError} />
            </CenteredContent>
        );
    }

    return (
        <>
            {saveError && (
                <CenteredContent>
                    <Alert variant='danger' title={saveError} />
                </CenteredContent>
            )}
            <TextArea
                style={{ height: '100%' }}
                aria-label="wsbrd-conf"
                value={content}
                onChange={onContentChange}
                onBlur={flushSaveQueue}
                resizeOrientation='vertical'

            />
        </>
    );
};

export default WSBRDConfEditorContent;
