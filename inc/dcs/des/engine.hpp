/**
 * \file dcs/des/engine.hpp
 *
 * \brief The core discrete-event simulator engine.
 *
 * Copyright (C) 2012       Distributed Computing System (DCS) Group,
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
 * \author Cosimo Anglano (cosimo.anglano@di.unipmn.it)
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */

#ifndef DCS_DES_ENGINE_HPP
#define DCS_DES_ENGINE_HPP


#include <algorithm>
#include <boost/smart_ptr.hpp>
#include <cmath>
#include <cstddef>
#include <dcs/assert.hpp>
#include <dcs/debug.hpp>
//#include <dcs/des/any_analyzable_statistic.hpp>
#include <dcs/des/any_statistic.hpp>
#include <dcs/des/base_analyzable_statistic.hpp>
#include <dcs/des/base_statistic.hpp>
#include <dcs/des/event.hpp>
#include <dcs/des/engine_context.hpp>
#include <dcs/des/event_list.hpp>
#include <dcs/des/event_source.hpp>
#include <dcs/exception.hpp>
#include <dcs/macro.hpp>
#include <dcs/math/traits/float.hpp>
//#include <functional>
#include <iostream>
//#include <queue>
//#include <vector>
#include <map>


namespace dcs { namespace des {

/**
 * \brief The core discrete-event simulator engine.
 *
 * \tparam RealT The type used for real numbers.
 *
 * This simulator engine does not provide by itself any output analysis method.
 * Use one of simulator engine classes derived from this one (e.g., see
 * \c dcs::des::batch_means::engine for the batch means output analysis).
 *
 * There are three types of events: core, custom, and auxiliary.
 *
 * <em>Core events</em> are (shown in firing order):
 * -# BEGIN-OF-SIMUALTION: fired just before starting the simulation.
 * -# SYSTEM-INITIALIZATION: fired just before running a simulation experiment for initializing the simulated system.
 * -# SYSTEM-FINALIZATION: fired just after running a simulation experiment for finalizing the simulated system.
 * -# END-OF-SIMUALTION: fired just after stopping the simulation.
 * .
 * <em>Custom events</em> are events defined by the user.
 *
 * <em>Auxiliary events</em> are the following events:
 * - BEFORE-OF-EVENT-FIRING: fired just before firing any custom event.
 * - AFTER-OF-EVENT-FIRING: fired just after firing any custom event.
 * .
 *
 * \author Cosimo Anglano, &lt;cosimo.anglano@mfn.unipmn.it&gt;
 * \author Marco Guazzone (marco.guazzone@gmail.com)
 */
template <typename RealT=double>
class engine
{
	private: typedef engine<RealT> self_type;
	public: typedef RealT real_type;
	public: typedef event<real_type> event_type;
	public: typedef engine_context<real_type> engine_context_type;
	public: typedef event_source<real_type> event_source_type;


	public: template <typename RT> friend ::std::ostream& operator<<(::std::ostream&, engine<RT> const&);


	/// The default constructor.
	public: engine()
	: evt_list_(),
	  ptr_bos_evt_src_(new event_source<RealT>("Begin of Simulation")),
	  ptr_eos_evt_src_(new event_source<RealT>("End of Simulation")),
	  ptr_bef_evt_src_(new event_source<RealT>("Before Event Firing")),
	  ptr_aef_evt_src_(new event_source<RealT>("After Event Firing")),
	  ptr_si_evt_src_(new event_source<RealT>("System Initialization")),
	  ptr_sf_evt_src_(new event_source<RealT>("System Finalization")),
	  sim_time_(0),
	  last_evt_time_(0),
	  end_of_sim_(true),
	  num_events_(0),
	  num_usr_events_(0),
	  mon_stats_()
	{
		// empty
	}

	/// Copy constructor
	private: engine(engine const& that)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( that );

		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-constructor not yet implemented." );
	}

	/// Copy assignment
	private: engine& operator=(engine const& rhs)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( rhs );

		DCS_EXCEPTION_THROW( ::std::runtime_error, "Copy-assignment not yet implemented." );
	}

	/// The destructor.
	public: virtual ~engine()
	{
	}

	//@{ Member functions

	/**
	 * \brief Add a new event to be scheduled at the specified time.
	 * \param ptr_src The event source which will fire the event.
	 * \param time The time the event is to be scheduled.
	 */
	//public: void schedule_event(boost::shared_ptr< event_source<RealT> > const& ptr_src, RealT time)
	public: boost::shared_ptr< event<RealT> > schedule_event(boost::shared_ptr< event_source<RealT> > const& ptr_src, RealT time)
	{
		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_src );

//		// pre: event source must be valid
//		DCS_ASSERT(
//				ptr_src->enabled(),
//				DCS_EXCEPTION_THROW( ::std::invalid_argument, "Cannot schedule events from a disabled event source." )
//			);

		if (!ptr_src->enabled())
		{
			::std::clog << "[Warning] Tried to schedule an event from the disabled event source '" << *ptr_src << "' at time: " << time << " (Clock: " << sim_time_ << ")" << ::std::endl;
			return boost::shared_ptr< event<RealT> >();
		}

		// check: only schedule future (or immediate) events
		if (time < sim_time_)
		{
			::std::clog << "[Warning] Fire time of event <" << *ptr_src << ", @ " << time << "> refers to the past: synched to current time (" << sim_time_ << ")." << ::std::endl;

			time = sim_time_;
		}

//		evt_list_.push(event<RealT>(ptr_src, time));
		boost::shared_ptr< event<RealT> > ptr_evt = ::boost::make_shared< event<RealT> >(ptr_src, sim_time_, time);
		evt_list_.push(ptr_evt);
		return ptr_evt;
	}


	/**
	 * \brief Add a new event to be scheduled at the specified time.
	 * \param ptr_src The event source which will fire the event.
	 * \param time The time the event is to be scheduled.
	 */
	public: template <typename T>
			boost::shared_ptr< event<RealT> > schedule_event(boost::shared_ptr< event_source<RealT> > const& ptr_src, RealT time, T const& state)
	{
		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_src );

//		// pre: event source must be valid
//		DCS_ASSERT(
//				ptr_src->enabled(),
//				DCS_EXCEPTION_THROW( ::std::invalid_argument, "Cannot schedule events from a disabled event source." )
//			);

		if (!ptr_src->enabled())
		{
			::std::clog << "[Warning] Tried to schedule an event from the disabled event source '" << *ptr_src << "' at time: " << time << " (Clock: " << sim_time_ << ")" << ::std::endl;
			return boost::shared_ptr< event<RealT> >();
		}

		// check: only future (or immediate) events can be scheduled
		if (time < sim_time_)
		{
			::std::clog << "[Warning] Fire time of event <" << *ptr_src << ", @ " << time << "> refers to the past: synched to current time (" << sim_time_ << ")." << ::std::endl;

			time = sim_time_;
		}

//		evt_list_.push(event<RealT>(ptr_src, time, state));
		boost::shared_ptr< event<RealT> > ptr_evt = ::boost::make_shared< event<RealT> >(ptr_src, sim_time_, time, state);
		evt_list_.push(ptr_evt);
		return ptr_evt;
	}


	public: void reschedule_event(boost::shared_ptr< event<RealT> > const& ptr_evt, RealT time)
	{
		// check: paranoid check
		DCS_DEBUG_ASSERT( ptr_evt );

		if (!ptr_evt->source().enabled())
		{
			::std::clog << "[Warning] Tried to reschedule an event from the disabled event source '" << ptr_evt->source() << "' at time: " << time << " (Clock: " << sim_time_ << ")" << ::std::endl;
			return;
		}

//		// pre: event source must be valid
//		DCS_ASSERT(
//				ptr_evt->source().enabled(),
//				DCS_EXCEPTION_THROW( ::std::invalid_argument, "Cannot reschedule events from a disabled event source." )
//			);

		if (time < sim_time_)
		{
			if (ptr_evt->fire_time() > sim_time_)
			{
				::std::clog << "[Warning] New fire time (" << time << ") of event " << *ptr_evt << "> refers to the past and will be adjusted to current time (" << sim_time_ << ")." << ::std::endl;
				time = sim_time_;
			}
			else
			{
				::std::clog << "[Warning] New fire time (" << time << ") of event " << *ptr_evt << "> refers to the past and will not be rescheduled." << ::std::endl;
				return;
			}
		}

		if (::dcs::math::float_traits<RealT>::essentially_equal(time, ptr_evt->fire_time()))
		{
			// Avoid to reschedule events with unchanged fire-time
			::std::clog << "[Warning] New fire time (" << time << ") of event " << *ptr_evt << "> is approximately equal to the old one and will not be rescheduled." << ::std::endl;
			return;
		}

		// Only future (or immediate) events are rescheduled
//		ptr_evt->fire_time(time);
//		evt_list_.touch(ptr_evt);
		evt_list_.erase(ptr_evt);
		ptr_evt->fire_time(time);
		evt_list_.push(ptr_evt);
	}


	/**
	 * \brief Return the event source related to the
	 *  <em>BEGIN-OF-SIMULATION</em> event.
	 * \return A reference to the <em>BEGIN-OF-SIMULATION</em> event source.
	 *
	 * The user may attach to this event source one or more event sinks in order
	 * to perform some preliminary operation before the simulation begins.
	 */
	public: event_source<RealT>& begin_of_sim_event_source()
	{
		return *ptr_bos_evt_src_;
	}


	/**
	 * \brief Return the event source related to the
	 *  <em>BEGIN-OF-SIMULATION</em> event.
	 * \return A const reference to the <em>BEGIN-OF-SIMULATION</em> event
	 *  source.
	 *
	 * The user may attach to this event source one or more event sinks in order
	 * to perform some preliminary operation before the simulation begins.
	 */
	public: event_source<RealT> const& begin_of_sim_event_source() const
	{
		return *ptr_bos_evt_src_;
	}


	/**
	 * \brief Return the event source related to the
	 *  <em>END-OF-SIMULATION</em> event.
	 * \return A reference to the <em>END-OF-SIMULATION</em> event source.
	 *
	 * The user may attach to this event source one or more event sinks in order
	 * to perform some final operation after the simulation ends.
	 */
	public: event_source<RealT>& end_of_sim_event_source()
	{
		return *ptr_eos_evt_src_;
	}


	/**
	 * \brief Return the event source related to the
	 *  <em>END-OF-SIMULATION</em> event.
	 * \return A const reference to the <em>END-OF-SIMULATION</em> event
	 *  source.
	 *
	 * The user may attach to this event source one or more event sinks in order
	 * to perform some final operation after the simulation ends.
	 */
	public: event_source<RealT> const& end_of_sim_event_source() const
	{
		return *ptr_eos_evt_src_;
	}


	/**
	 * \brief Return the event source related to the
	 *  <em>BEFORE-OF-EVENT-FIRING</em> event.
	 * \return A reference to the <em>BEFORE-OF-EVENT-FIRING</em> event source.
	 *
	 * The user may attach to this event source one or more event sinks in order
	 * to perform some final operation before the event is fired.
	 */
	public: event_source<RealT>& before_of_event_firing_source()
	{
		return *ptr_bef_evt_src_;
	}


	/**
	 * \brief Return the event source related to the
	 *  <em>BEFORE-OF-EVENT-FIRING</em> event.
	 * \return A const reference to the <em>BEFORE-OF-EVENT-FIRING</em> event
	 *  source.
	 *
	 * The user may attach to this event source one or more event sinks in order
	 * to perform some operation before the event is fired.
	 */
	public: event_source<RealT> const& before_of_event_firing_source() const
	{
		return *ptr_bef_evt_src_;
	}


	/**
	 * \brief Return the event source related to the
	 *  <em>AFTER-OF-EVENT-FIRING</em> event.
	 * \return A reference to the <em>AFTER-OF-EVENT-FIRING</em> event source.
	 *
	 * The user may attach to this event source one or more event sinks in order
	 * to perform some final operation after the event is fired.
	 */
	public: event_source<RealT>& after_of_event_firing_source()
	{
		return *ptr_aef_evt_src_;
	}


	/**
	 * \brief Return the event source related to the
	 *  <em>AFTER-OF-EVENT-FIRING</em> event.
	 * \return A const reference to the <em>AFTER-OF-EVENT-FIRING</em> event
	 *  source.
	 *
	 * The user may attach to this event source one or more event sinks in order
	 * to perform some operation after the event is fired.
	 */
	public: event_source<RealT> const& after_of_event_firing_source() const
	{
		return *ptr_aef_evt_src_;
	}


	/**
	 * \brief Return the event source related to the
	 *  <em>SYSTEM-INITIALIZATION</em> event.
	 * \return A reference to the <em>SYSTEM-INITIALIZATION</em> event source.
	 *
	 * The user may attach to this event source one or more event sinks in order
	 * to perform some operation during the initialization of the simulated
	 * system
	 */
	public: event_source<RealT>& system_initialization_event_source()
	{
		return *ptr_si_evt_src_;
	}


	/**
	 * \brief Return the event source related to the
	 *  <em>SYSTEM-INITIALIZATION</em> event.
	 * \return A const reference to the <em>SYSTEM-INITIALIZATION</em> event
	 *  source.
	 *
	 * The user may attach to this event source one or more event sinks in order
	 * to perform some operation during the initialization of the simulated
	 * system
	 */
	public: event_source<RealT> const& system_initialization_event_source() const
	{
		return *ptr_si_evt_src_;
	}


	/**
	 * \brief Return the event source related to the
	 *  <em>SYSTEM-FINALIZATION</em> event.
	 * \return A reference to the <em>SYSTEM-FINALIZATION</em> event source.
	 *
	 * The user may attach to this event source one or more event sinks in order
	 * to perform some operation during the finalization of the simulated
	 * system
	 */
	public: event_source<RealT>& system_finalization_event_source()
	{
		return *ptr_sf_evt_src_;
	}


	/**
	 * \brief Return the event source related to the
	 *  <em>SYSTEM-FINALIZATION</em> event.
	 * \return A const reference to the <em>SYSTEM-FINALIZATION</em> event
	 *  source.
	 *
	 * The user may attach to this event source one or more event sinks in order
	 * to perform some operation during the finalization of the simulated
	 * system
	 */
	public: event_source<RealT> const& system_finalization_event_source() const
	{
		return *ptr_sf_evt_src_;
	}


	/**
	 * \brief Set a statistic to be analyzed.
	 * \param ptr_stat A pointer to the statistic to be analyzed.
	 */
	public: void analyze_statistic(boost::shared_ptr< base_analyzable_statistic<RealT> > const& ptr_stat)
	{
		//mon_stats_.push_back(ptr_stat);
		mon_stats_[ptr_stat] = ptr_stat->steady_state_entered();

		if (!end_of_sim_)
		{
			ptr_stat->initialize_for_experiment();
		}

//		if (
//			!(ptr_mon_stat_.get()) ||
//			ptr_mon_stat_->target_relative_precision() > ptr_stat->target_relative_precision()
//		) {
//			ptr_mon_stat_ = ptr_stat;
//		}
	}


	public: void remove_statistic(boost::shared_ptr< base_analyzable_statistic<RealT> > const& ptr_stat)
	{
		// pre: ptr_stat must be a valid pointer
		DCS_ASSERT(
				ptr_stat,
				DCS_EXCEPTION_THROW( ::std::invalid_argument, "Invalid statistic pointer." )
			);
		// pre: ptr_stat must be a pointer to an analyzed stat
		DCS_ASSERT(
				mon_stats_.count(ptr_stat.get()) > 0,
				DCS_EXCEPTION_THROW( ::std::invalid_argument, "Statistic not analyzed." )
			);

		mon_stats_.erase(ptr_stat);
	}


	/// Deregister all the analyzable statistics.
	//public: void ignore_statistics()
	public: void remove_statistics()
	{
		mon_stats_.clear();
	}


//	/**
//	 * \brief Set a statistic to be analyzed.
//	 * \param ptr_stat A pointer to the statistic to be analyzed.
//	 */
//	public: boost::shared_ptr< base_analyzable_statistic<RealT> > make_analyzable_statistic(any_statistic<RealT> const& stat)
//	{
//		return do_make_analyzable_statistic(stat);
//	}


	/**
	 * \brief Set a statistic to be analyzed.
	 * \param ptr_stat A pointer to the statistic to be analyzed.
	 */
	public: template <typename StatisticT>
		boost::shared_ptr< base_analyzable_statistic<RealT> > make_analyzable_statistic(StatisticT stat)
	{
		boost::shared_ptr< base_analyzable_statistic<RealT> > ptr_stat;
		ptr_stat = do_make_analyzable_statistic(::dcs::des::make_any_statistic(stat));
		analyze_statistic(ptr_stat);
		return ptr_stat;
	}


//	/**
//	 * \brief Return the simulated time to date.
//	 * \return The simulated time to date.
//	 *
//	 * \deprecated Use simulated_time()
//	 */
//	public: RealT sim_time() const
//	{
//		return sim_time_;
//	}


	/**
	 * \brief Return the simulated time to date.
	 * \return The simulated time to date.
	 */
	public: RealT simulated_time() const
	{
		return sim_time_;
	}


	/**
	 * \brief Return the simulated time of the last fired event.
	 * \return The simulated time of the last fired event.
	 */
	public: RealT last_event_time() const
	{
		return last_evt_time_;
	}


	/**
	 * \brief Tell if simulation is done.
	 * \return \c true if simulation is done; \c false otherwise.
	 */
	public: bool end_of_simulation() const
	{
		return end_of_sim_;
	}


//	// Maybe useless
//	protected: std::size_t num_events() const
//	{
//		return num_events_;
//	}


	/**
	 * \brief Run the simulation and terminate it according to either the
	 *  user-defined stopping conditions or when there are no more events
	 *  to be fired.
	 */
	public: void run()
	{
		end_of_sim_ = false;

		do_run();

		end_of_sim_ = true;
	}


	public: void advance()
	{
		if (!end_of_sim_ && !evt_list_.empty())
		{
			engine_context<RealT> ctx(this);

			fire_next_event(ctx);

			monitor_statistics();
		}
	}


	/// Stop the simulation just now.
	public: void stop_now()
	{
		do_stop_now();

//		// Immediately (schedule and) fire the END-OF-SIMULATION event
//		engine_context<RealT> ctx(this);
//		fire_immediate_event(ptr_eos_evt_src_, ctx);

		end_of_sim_ = true;
	}



	/**
	 * \brief Stop the simulation at the specified time.
	 * \param time The time at which the simulation is to be stopped.
	 */
	public: void stop_at_time(RealT time)
	{
		do_stop_at_time(time);
	}


	protected: virtual bool is_internal_event(event<RealT> const& evt) const
	{
		return evt.source() == *ptr_bos_evt_src_
			   || evt.source() == *ptr_eos_evt_src_
			   || evt.source() == *ptr_bef_evt_src_
			   || evt.source() == *ptr_aef_evt_src_;
	}


	protected: virtual void do_stop_now()
	{
//		stop_at_time(sim_time_);

//		end_of_sim_ = true;
	}


	protected: virtual void do_stop_at_time(RealT time)
	{
		// preconditions
		DCS_ASSERT(
			time >= sim_time_,
			throw std::logic_error("[dcs::des::engine::stop_at_time] Cannot stop the simulation at a past time.")
		);

		schedule_event(ptr_eos_evt_src_, time);
	}


	protected: void reset()
	{
		sim_time_ = last_evt_time_
				  = RealT(0);

		num_events_ = std::size_t(0);

		end_of_sim_ = false;

		evt_list_.clear();

		// NO! This clash with specialized engines (like independent replications) which call this method.
		// Reset statistics
		//reset_statistics();
	}


	protected: virtual void prepare_simulation(engine_context<RealT>& ctx)
	{
		// Clear simulation state
		reset();

		// Reset statistics
		reset_statistics();

//		// Insert a new begin-of-simulation event
//		schedule_event(ptr_bos_evt_src_, RealT(0));
//		// Fire the begin of simulation event
//		fire_next_event(ctx);
		// Immediately (schedule and) fire the BEGIN-OF-SIMULATION event
		fire_immediate_event(ptr_bos_evt_src_, ctx);
	}


	protected: virtual void finalize_simulation(engine_context<RealT>& ctx)
	{
		DCS_MACRO_SUPPRESS_UNUSED_VARIABLE_WARNING( ctx );

		if (!end_of_sim_)
		{
			// Clear the possibly non-empty event list
			//evt_list_.clear();

			// Fire the end of simulation event
//			stop_now();
//			fire_next_event(ctx);

			end_of_sim_ = true;
		}

		evt_list_.clear();

		// Immediately (schedule and) fire the END-OF-SIMULATION event
//		engine_context<RealT> ctx(this);
		fire_immediate_event(ptr_eos_evt_src_, ctx);
	}


	protected: virtual void initialize_simulated_system(engine_context<RealT>& ctx)
	{
//		// Schedule the system-initialization event now...
//		schedule_event(ptr_si_evt_src_, sim_time_);
//		// ... and immediately fire it
//		fire_next_event(ctx);
		// Immediately (schedule and) fire the SYSTEM-INITIALIZATION event
		fire_immediate_event(ptr_si_evt_src_, ctx);
	}


	protected: virtual void finalize_simulated_system(engine_context<RealT>& ctx)
	{
//		// Schedule the system-initialization event now...
//		schedule_event(ptr_sf_evt_src_, sim_time_);
//		// ... and immediately fire it
//		fire_next_event(ctx);
		// Immediately (schedule and) fire the SYSTEM-FINALIZATION event
		fire_immediate_event(ptr_sf_evt_src_, ctx);
	}


	protected: void fire_next_event(engine_context<RealT>& ctx)
	{
		if (!evt_list_.empty())
		{
			//event<RealT> cur_evt = evt_list_.top();
			boost::shared_ptr< event<RealT> > ptr_cur_evt = evt_list_.top();
			evt_list_.pop();

			DCS_DEBUG_ASSERT( ptr_cur_evt );

			if (!ptr_cur_evt->source().enabled())
			{
				::std::clog << "[Warning] Event '" << *ptr_cur_evt << "' will not be fired since its source is disabled." << ::std::endl;
				return;
			}

			DCS_DEBUG_ASSERT( ptr_cur_evt->fire_time() >= sim_time_ );

			++num_events_;

			//if (!is_internal_event(cur_evt))
			if (!is_internal_event(*ptr_cur_evt))
			{
				++num_usr_events_;
			}

			// Advance effective and simulated elapsed time
			//RealT cur_time = cur_evt.fire_time();
			RealT cur_time = ptr_cur_evt->fire_time();
			sim_time_ = cur_time;

			//DCS_DEBUG_TRACE_L(1, "Firing EVENT #" << num_events_ << ": " << cur_evt );
			DCS_DEBUG_TRACE_L(1, "Firing EVENT #" << num_events_ << ": " << *ptr_cur_evt );

			// Firing the before-event-firing event
			if (!ptr_bef_evt_src_->empty())
			{
				//event<RealT>(ptr_bef_evt_src_, sim_time_, cur_evt).fire(ctx);
				make_internal_event(ptr_bef_evt_src_, *ptr_cur_evt).fire(ctx);
				++num_events_;
			}

			//cur_evt.fire(ctx);
			ptr_cur_evt->fire(ctx);

			// Firing the after-event-firing event
			if (!ptr_aef_evt_src_->empty())
			{
				//event<RealT>(ptr_aef_evt_src_, sim_time_, cur_evt).fire(ctx);
				make_internal_event(ptr_aef_evt_src_, *ptr_cur_evt).fire(ctx);
				++num_events_;
			}

			last_evt_time_ = cur_time;

			// Check for the end-of-simulation event
			//if (cur_evt.source() == *ptr_eos_evt_src_)
			if (ptr_cur_evt->source() == *ptr_eos_evt_src_)
			{
				end_of_sim_ = true;
			}
		}
	}


	protected: void fire_immediate_event(boost::shared_ptr< event_source<RealT> > const& ptr_src, engine_context<RealT>& ctx)
	{
		event<RealT> cur_evt(ptr_src, sim_time_, sim_time_);

		if (!cur_evt.source().enabled())
		{
			::std::clog << "[Warning] Immediate event '" << cur_evt << "' will not be fired since its source is disabled." << ::std::endl;
			return;
		}

		++num_events_;

		if (!is_internal_event(cur_evt))
		{
			++num_usr_events_;
		}

		DCS_DEBUG_TRACE_L(1, "Firing (immediate) EVENT #" << num_events_ << ": " << cur_evt );

		// Firing the before-event-firing event
		if (!ptr_bef_evt_src_->empty())
		{
			make_internal_event(ptr_bef_evt_src_, cur_evt).fire(ctx);
			++num_events_;
		}

		cur_evt.fire(ctx);

		// Firing the after-event-firing event
		if (!ptr_aef_evt_src_->empty())
		{
			make_internal_event(ptr_aef_evt_src_, cur_evt).fire(ctx);
			++num_events_;
		}

		last_evt_time_ = sim_time_;

		// Check for the end-of-simulation event
		if (cur_evt.source() == *ptr_eos_evt_src_)
		{
			end_of_sim_ = true;
		}
	}


	protected: template <typename T>
		void fire_immediate_event(boost::shared_ptr< event_source<RealT> > const& ptr_src, engine_context<RealT>& ctx, T const& state)
	{
		event<RealT> cur_evt(ptr_src, sim_time_, sim_time_, state);

		if (!cur_evt.source().enabled())
		{
			::std::clog << "[Warning] Immediate event '" << cur_evt << "' will not be fired since its source is disabled." << ::std::endl;
			return;
		}

		++num_events_;

		if (!is_internal_event(cur_evt))
		{
			++num_usr_events_;
		}

		DCS_DEBUG_TRACE_L(1, "Firing (immediate) EVENT #" << num_events_ << ": " << cur_evt );

		// Firing the before-event-firing event
		if (!ptr_bef_evt_src_->empty())
		{
			make_internal_event(ptr_bef_evt_src_, cur_evt).fire(ctx);
			++num_events_;
		}

		cur_evt.fire(ctx);

		// Firing the after-event-firing event
		if (!ptr_aef_evt_src_->empty())
		{
			make_internal_event(ptr_aef_evt_src_, cur_evt).fire(ctx);
			++num_events_;
		}

		last_evt_time_ = sim_time_;

		// Check for the end-of-simulation event
		if (cur_evt.source() == *ptr_eos_evt_src_)
		{
			end_of_sim_ = true;
		}
	}


	protected: void reset_statistics()
	{
		if (mon_stats_.empty())
		{
			return;
		}

		typedef typename std::map< boost::shared_ptr< base_analyzable_statistic<RealT> >,bool>::iterator analyzable_statistic_iterator;

		const analyzable_statistic_iterator end_it = mon_stats_.end();
		for (analyzable_statistic_iterator it = mon_stats_.begin();
			 it != end_it;
			 ++it)
		{
			boost::shared_ptr< base_analyzable_statistic<RealT> > ptr_stat(it->first);

			ptr_stat->reset();
		}
	}


	protected: void monitor_statistics()
	{
//		// Check if precision has been reached.
//		DCS_DEBUG_TRACE("Checking for precision -- reached: " << ptr_mon_stat_->relative_precision() << ", wanted: " << ptr_mon_stat_->target_relative_precision());
//
//		//if (ptr_stats->target_precision_reached())
//		if (ptr_mon_stat_->target_precision_reached())
//		{
//			DCS_DEBUG_TRACE("Target precision reached: " << ptr_mon_stat_->relative_precision() << ", wanted: " << ptr_mon_stat_->target_relative_precision());
//
//			end_of_sim_ = true;
//		}

		if (mon_stats_.empty())
		{
			return;
		}

		typedef typename std::map< boost::shared_ptr<base_analyzable_statistic<RealT> >,bool>::iterator analyzable_statistic_iterator;

		const analyzable_statistic_iterator end_it = mon_stats_.end();
		bool prec_reached = true;
		for (analyzable_statistic_iterator it = mon_stats_.begin();
			 it != end_it;
			 ++it)
		{
			boost::shared_ptr< base_analyzable_statistic<RealT> > ptr_stat(it->first);

			DCS_DEBUG_TRACE_L(1,"Stat " << ptr_stat.get() << ">> Checking for precision -- reached: " << ptr_stat->relative_precision() << ", wanted: " << ptr_stat->target_relative_precision());

			if (!it->second && ptr_stat->steady_state_entered())
			{
				it->second = true;
				ptr_stat->steady_state_enter_time(sim_time_);
			}

			// Check if precision has been reached.
			if (ptr_stat->enabled() && !ptr_stat->target_precision_reached())
			{
				DCS_DEBUG_TRACE_L(1,"Target precision NOT reached.");
				prec_reached = false;
				//break; //NO! We must loop for every stat since we also set the steady-state enter time (see above)
			}
		}


		if (prec_reached)
		{
			DCS_DEBUG_TRACE_L(1,"Target precision reached for all statistics (or possibly some of them are disabled).");

			end_of_sim_ = true;
		}
	}


	protected: std::map< boost::shared_ptr< base_analyzable_statistic<RealT> >,bool>& monitored_statistics()
	{
		return mon_stats_;
	}


	protected: std::map< boost::shared_ptr< base_analyzable_statistic<RealT> >,bool> const& monitored_statistics() const
	{
		return mon_stats_;
	}


	protected: event_list< event<RealT> >& future_event_list()
	{
		return evt_list_;
	}


	protected: event_list< event<RealT> > const& future_event_list() const
	{
		return evt_list_;
	}


	protected: void simulated_time(RealT value)
	{
		sim_time_ = value;
	}


	protected: void end_of_simulation(bool value)
	{
		end_of_sim_ = value;
	}


	protected: virtual void print(::std::ostream& os) const
	{
		os << "<(generic engine)>";
	}


	protected: event<RealT> make_internal_event(boost::shared_ptr< event_source<RealT> > const& ptr_evt_src, event<RealT> const& embedded_evt)
	{
		return event<RealT>(ptr_evt_src, sim_time_, sim_time_, embedded_evt);
	}


	private: virtual void do_run() = 0;
//	protected: virtual void do_run(engine_context<RealT>& ctx)
//	{
//		while (!end_of_sim_ && !evt_list_.empty())
//		{
//			DCS_DEBUG_TRACE_L(1, "Simulation time: " << sim_time_ );
//
//			fire_next_event(ctx);
//
//			monitor_statistics();
//		}
//
//		if (end_of_sim_)
//		{
//			evt_list_.clear();
//		}
//	}

	private: virtual boost::shared_ptr< base_analyzable_statistic<RealT> > do_make_analyzable_statistic(any_statistic<RealT> const& stat) = 0;

	//@} Member functions


	//@{ Member variables

	private: event_list< event<RealT> > evt_list_; ///< The event list.
	private: boost::shared_ptr< event_source<RealT> > ptr_bos_evt_src_; ///< The source of the begin-of-simulation event
	private: boost::shared_ptr< event_source<RealT> > ptr_eos_evt_src_; ///< The source of the end-of-simulation event
	private: boost::shared_ptr< event_source<RealT> > ptr_bef_evt_src_; ///< The source of the before-event-firing event
	private: boost::shared_ptr< event_source<RealT> > ptr_aef_evt_src_; ///< The source of the after-event-firing event
	private: boost::shared_ptr< event_source<RealT> > ptr_si_evt_src_; ///< The source of the system-initialization event
	private: boost::shared_ptr< event_source<RealT> > ptr_sf_evt_src_; ///< The source of the system-finalization event
	private: RealT sim_time_; ///< The simulated time (does not include pause time).
	private: RealT last_evt_time_; ///< The time of the last fired event.
	private: bool end_of_sim_; ///< Tell if simulation is done.
	private: std::size_t num_events_; ///< The total number of events (including the ones scheduled by the engine itself).
	private: std::size_t num_usr_events_; ///< The total number of events (including the ones scheduled by the engine itself).
	private: std::map< boost::shared_ptr< base_analyzable_statistic<RealT> >,bool> mon_stats_;
	//private: boost::shared_ptr< base_analyzable_statistic<RealT> > ptr_mon_stat_;

	//@} Member variables
}; // engine


template <typename RealT>
inline
::std::ostream& operator<<(::std::ostream& os, engine<RealT> const& eng)
{
	eng.print(os);

	return os;
}


}} // Namespace dcs::des


#endif // DCS_DES_ENGINE_HPP
