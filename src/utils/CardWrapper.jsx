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

import { Card, CardHeader, CardTitle, CardBody } from "@patternfly/react-core";

const CardWrapper = ({ title, children }) => {
    return (
        <Card style={{ height: '100%' }}>
            <CardHeader>
                <CardTitle>{title}</CardTitle>
            </CardHeader>
            <CardBody>
                {children}
            </CardBody>
        </Card>
    );
};

export default CardWrapper;
