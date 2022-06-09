/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/**
 * Since there is no way so far to add a blend_read_data callback in node type
 * (because node->typeinfo is not set yet when reading node data), we make
 * read callbacks for individual nodes available here.
 */

#pragma once

struct BlendDataReader;
struct bNode;

namespace blender::nodes::node_geo_open_mfx_cc {
void node_read_data(BlendDataReader *reader, bNode *node);
}
