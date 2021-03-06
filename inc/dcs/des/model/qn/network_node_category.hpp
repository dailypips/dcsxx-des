/**
 * \file src/dcs/des/model/qn/network_node_category.hpp
 *
 * \brief Categories for queueing network nodes.
 *
 * Copyright (C) 2009-2012  Distributed Computing System (DCS) Group,
 *                          Computer Science Institute,
 *                          Department of Science and Technological Innovation,
 *                          University of Piemonte Orientale,
 *                          Alessandria (Italy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */

#ifndef DCS_DES_MODEL_QN_NETWORK_NODE_CATEGORY_HPP
#define DCS_DES_MODEL_QN_NETWORK_NODE_CATEGORY_HPP


namespace dcs { namespace des { namespace model { namespace qn {

enum network_node_category
{
	delay_station_node_category,
//	fcfs_station_node_category,
	source_node_category,
	service_station_node_category,
	//population_station_category,
	sink_node_category
};

}}}} // Namespace dcs::des::model::qn


#endif // DCS_DES_MODEL_QN_NETWORK_NODE_CATEGORY_HPP
