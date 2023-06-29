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

import { useState, useContext } from "react";
import {
    Button,
    Card,
    CardActions,
    CardBody,
    CardHeader,
    CardTitle,
    DescriptionList,
    DescriptionListDescription,
    DescriptionListGroup,
    DescriptionListTerm,
    Dropdown,
    DropdownItem,
    DropdownPosition,
    DropdownSeparator,
    Icon,
    KebabToggle,
    Spinner
} from "@patternfly/react-core";
import ExclamationCircleIcon from '@patternfly/react-icons/dist/esm/icons/exclamation-circle-icon';
import CheckCircleIcon from '@patternfly/react-icons/dist/esm/icons/check-circle-icon';
import PauseIcon from '@patternfly/react-icons/dist/esm/icons/pause-icon';
import cockpit from 'cockpit';
import { AppContext } from "../app";

const _ = cockpit.gettext;

const WSBRDStatus = () => {
    const [isOpen, setIsOpen] = useState(false);
    const { active, setActive, setLoading } = useContext(AppContext);

    const onDropdownItemClick = (value) => {
        setLoading(true);

        cockpit.spawn(["systemctl", value, "wisun-borderrouter.service"], { superuser: "require" })
            .then(() => {
                setActive(undefined);
            })
            .catch((err) => {
                console.log(err);
                setActive(undefined);
            });
        setIsOpen(false);
    };

    const onLogsClick = () => {
        cockpit.jump("/system/logs#/?prio=debug&service=wisun-borderrouter.service");
    };

    const getStatusText = () => {
        if (active === undefined) {
            return 'Loading...';
        }
        if (active === null) {
            return <span>Could not retrieve service status</span>;
        }
        if (active) {
            return 'Active';
        }
        return 'Inactive';
    };

    const getStatusIcon = () => {
        if (active === undefined) {
            return <Spinner isSVG />;
        }
        if (active === null) {
            return (
                <Icon status="danger">
                    <ExclamationCircleIcon />
                </Icon>
            );
        }
        if (active) {
            return (
                <Icon status="success">
                    <CheckCircleIcon />
                </Icon>
            );
        }
        return (
            <Icon status="info">
                <PauseIcon />
            </Icon>
        );
    };

    const dropdownItems = [
        <DropdownItem key="start" component="button" onClick={() => onDropdownItemClick('start')}>Start</DropdownItem>,
        <DropdownItem
            key="restart"
            component="button"
            onClick={() => onDropdownItemClick('restart')}
        >Restart
        </DropdownItem>,
        <DropdownSeparator key="separator" />,
        <DropdownItem key="stop" component="button" onClick={() => onDropdownItemClick('stop')}>
            Stop
        </DropdownItem>
    ];

    return (
        <Card>
            <CardHeader>
                <CardTitle>
                    Wi-SUN Border Router Service
                </CardTitle>
                <CardActions>
                    <Dropdown
                        dropdownItems={dropdownItems}
                        toggle={<KebabToggle onToggle={() => setIsOpen(!isOpen)} />}
                        isOpen={isOpen}
                        position={DropdownPosition.right}
                        isPlain
                    />
                </CardActions>
            </CardHeader>
            <CardBody>
                <DescriptionList isHorizontal>
                    <DescriptionListGroup>
                        <DescriptionListTerm>Status</DescriptionListTerm>
                        <DescriptionListDescription>
                            <DescriptionList isHorizontal isFluid>
                                <DescriptionListGroup>
                                    <DescriptionListTerm icon={getStatusIcon()} />
                                    <DescriptionListDescription>
                                        {getStatusText()}
                                    </DescriptionListDescription>
                                </DescriptionListGroup>
                                <DescriptionListGroup>
                                    <DescriptionListDescription>
                                        <Button variant="link" isSmall isInline onClick={onLogsClick}>
                                            Check wsbrd logs
                                        </Button>
                                    </DescriptionListDescription>
                                </DescriptionListGroup>
                            </DescriptionList>
                        </DescriptionListDescription>
                    </DescriptionListGroup>
                </DescriptionList>
            </CardBody>
        </Card>
    );
};

export default WSBRDStatus;
