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
    Spinner,
    Form,
    FormGroup,
    Radio
} from "@patternfly/react-core";
import ExclamationCircleIcon from '@patternfly/react-icons/dist/esm/icons/exclamation-circle-icon';
import CheckCircleIcon from '@patternfly/react-icons/dist/esm/icons/check-circle-icon';
import PauseIcon from '@patternfly/react-icons/dist/esm/icons/pause-icon';
import cockpit from 'cockpit';
// Added: reuse shared context and service mapping
import { AppContext, SERVICE_LABELS, SERVICE_SHORT_NAMES, SERVICE_UNITS } from "../app";

const _ = cockpit.gettext;

const WSBRDStatus = () => {
    const [isOpen, setIsOpen] = useState(false);
    // Added: pull selection and service data from context
    const {
        active,
        setLoading,
        services,
        selectedService,
        setSelectedService,
        refreshServices
    } = useContext(AppContext);
    // Added: resolve the systemd unit for the chosen service
    const serviceUnit = selectedService ? SERVICE_UNITS[selectedService] : null;
    const selectedServiceName = selectedService
        ? SERVICE_SHORT_NAMES[selectedService] // Added: obtain a short label for messaging
        : null; // Added: fall back to a neutral label when no service is selected
    const installedServices = Object.entries(services)
        .filter(([, service]) => service.installed); // Added: list installed services for the selector
    const multipleServicesInstalled =
        installedServices.length > 1; // Added: detect when both services are available

    const onDropdownItemClick = (value) => {
        if (!serviceUnit) { // Added: skip actions when no service is selected
            setIsOpen(false); // Added: close the dropdown without performing an action
            return; // Added: exit early when no service is chosen
        }
        setLoading(true);

        cockpit.spawn(
            ["systemctl", value, serviceUnit],
            { superuser: "require" }
        ) // Added: control the chosen service unit
            .then(() => {
                refreshServices(); // Added: refresh status after a successful command
            })
            .catch((err) => {
                console.log(err);
                refreshServices(); // Added: refresh even after failures to resync the UI
            });
        setIsOpen(false);
    };

    const onLogsClick = () => {
        if (!serviceUnit) { // Added: guard log navigation when no service is chosen
            return; // Added: exit if the user has not selected a service
        }
        cockpit.jump(`/system/logs#/?prio=debug&service=${serviceUnit}`); // Added: open logs for the selected service
    };

    const getStatusText = () => {
        if (!serviceUnit) { // Added: prompt the user to choose a service before showing status
            return 'Select a service to view its status'; // Added: instruct the user to pick a service
        }
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
        if (!serviceUnit) { // Added: hide status icon when no service is selected
            return null; // Added: render nothing until a service is chosen
        }
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
                        isDisabled={!serviceUnit} // Added: disable actions until a service is selected
                    />
                </CardActions>
            </CardHeader>
            <CardBody>
                {multipleServicesInstalled && ( // Added: show service selection when both units are available
                    <Form isHorizontal> {/* Added: arrange the selection radios within a horizontal form */}
                        <FormGroup label="Service" fieldId="ws-service-selection"> {/* Added: label the radio group */}
                            <Radio
                                id="ws-service-linux"
                                name="ws-service-selection"
                                label={SERVICE_LABELS.linux}
                                isChecked={selectedService === 'linux'}
                                isDisabled={!services.linux.installed}
                                onChange={() => setSelectedService('linux')}
                            />
                            <Radio
                                id="ws-service-soc"
                                name="ws-service-selection"
                                label={SERVICE_LABELS.soc}
                                isChecked={selectedService === 'soc'}
                                isDisabled={!services.soc.installed}
                                onChange={() => setSelectedService('soc')}
                            />
                        </FormGroup>
                    </Form>
                )}
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
                                        <Button // Added: expanded props to satisfy lint requirements
                                            variant="link"
                                            isSmall
                                            isInline
                                            onClick={onLogsClick}
                                            isDisabled={!serviceUnit}
                                        >
                                            {`Check ${selectedServiceName || 'service'} logs`}
                                            {/* Added: align copy with the selected service */}
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
