/**
 * \file dcs/des/fwd.hpp
 *
 * \brief Forward declaration of Discrete-Event Simulation classes.
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

#ifndef DCS_DES_FWD_HPP
#define DCS_DES_FWD_HPP


namespace dcs { namespace des {

template <typename RealT>
class engine;

template <typename RealT>
class engine_context;

template <typename RealT>
class event;

template <typename RealT>
class event_source;

}} // Namespace dcs::des


#endif // DCS_DES_FWD_HPP
