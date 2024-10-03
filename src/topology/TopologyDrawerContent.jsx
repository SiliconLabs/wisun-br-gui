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
    DrawerPanelContent,
    DrawerHead,
    DrawerActions,
    DrawerCloseButton,
    DrawerPanelBody,
    DescriptionList,
    DescriptionListGroup,
    DescriptionListTerm,
    DescriptionListDescription,
    Title,
    FlexItem,
    Flex
} from "@patternfly/react-core";
import cockpit from "cockpit";
import CenteredContent from "../utils/CenteredContent";

const _ = cockpit.gettext;

const NodeRolesString = ["Border Router", "FFN", "LFN"];

const TopologyDrawerContent = ({ closeDrawer, selectedNode, graph }) => {
    return (
        <DrawerPanelContent isResizable>
            <DrawerHead>
                <Title headingLevel="h3">Details</Title>
                <DrawerActions>
                    <DrawerCloseButton onClick={closeDrawer} />
                </DrawerActions>
            </DrawerHead>
            <DrawerPanelBody>
                {
                    selectedNode === null
                        ? (
                            <CenteredContent>
                                <Title headingLevel="h2">Select a node to access its details</Title>
                            </CenteredContent>
                        )
                        : (
                            <Flex direction={{ default: 'column' }} spaceItems={{ default: 'spaceItemsMd' }}>
                                <FlexItem>
                                    <DescriptionList isHorizontal>
                                        <DescriptionListGroup>
                                            <DescriptionListTerm>Node Type</DescriptionListTerm>
                                            <DescriptionListDescription>
                                                {
                                                    NodeRolesString[selectedNode.nodeRole]
                                                }
                                            </DescriptionListDescription>
                                        </DescriptionListGroup>
                                        <DescriptionListGroup>
                                            <DescriptionListTerm>IPv6 address</DescriptionListTerm>
                                            <DescriptionListDescription>
                                                {selectedNode.ipv6}
                                            </DescriptionListDescription>
                                        </DescriptionListGroup>
                                        {
                                            selectedNode.parentIPv6 &&
                                            <DescriptionListGroup>
                                                <DescriptionListTerm>Parent IPv6 address</DescriptionListTerm>
                                                <DescriptionListDescription>
                                                    {selectedNode.parentIPv6}
                                                </DescriptionListDescription>
                                            </DescriptionListGroup>
                                        }
                                        <DescriptionListGroup>
                                            <DescriptionListTerm>Children</DescriptionListTerm>
                                            <DescriptionListDescription>
                                                {
                                                    graph.nodes
                                                        .filter((n) => n.parentIPv6 !== undefined &&
                                                            n.parentIPv6.localeCompare(selectedNode.ipv6) === 0).length
                                                }
                                            </DescriptionListDescription>
                                        </DescriptionListGroup>
                                        <DescriptionListGroup>
                                            <DescriptionListTerm>Hop Level</DescriptionListTerm>
                                            <DescriptionListDescription>
                                                {selectedNode.level}
                                            </DescriptionListDescription>
                                        </DescriptionListGroup>
                                    </DescriptionList>
                                </FlexItem>
                            </Flex>
                        )
                }
            </DrawerPanelBody>
        </DrawerPanelContent>
    );
};

export default TopologyDrawerContent;
