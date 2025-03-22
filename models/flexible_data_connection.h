/*
 *  flexible_data_connection.h
 *
 *  This file is part of NEST.
 *
 *  Copyright (C) 2004 The NEST Initiative
 *
 *  NEST is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  NEST is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NEST.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef FLEXIBLE_DATA_CONNECTION_H
#define FLEXIBLE_DATA_CONNECTION_H

#include "connection.h"

namespace nest
{

/* BeginUserDocs: synapse, gap junction

Short description
+++++++++++++++++

Synapse type for flexible data connections

Description
+++++++++++

``flexible_data_connection`` is a connector to create flexible
data connections.

The value of the parameter ``delay`` is ignored for connections of
type ``flexible_data_connection``.

See also [1]_, [2]_.

Sends
+++++

FlexibleDataEvent

EndUserDocs */

void register_flexible_data_connection( const std::string& name );

template < typename targetidentifierT >
class flexible_data_connection : public Connection< targetidentifierT >
{

public:
  // this line determines which common properties to use
  typedef CommonSynapseProperties CommonPropertiesType;
  typedef Connection< targetidentifierT > ConnectionBase;

  static constexpr ConnectionModelProperties properties = ConnectionModelProperties::SUPPORTS_WFR;

  /**
   * Default Constructor.
   * Sets default values for all parameters. Needed by GenericConnectorModel.
   */
  flexible_data_connection()
    : ConnectionBase()
    , weight_( 1.0 )
  {
  }

  SecondaryEvent* get_secondary_event();

  // Explicitly declare all methods inherited from the dependent base
  // ConnectionBase. This avoids explicit name prefixes in all places these
  // functions are used. Since ConnectionBase depends on the template parameter,
  // they are not automatically found in the base class.
  using ConnectionBase::get_delay_steps;
  using ConnectionBase::get_rport;
  using ConnectionBase::get_target;

  void
  check_connection( Node& s, Node& t, size_t receptor_type, const CommonPropertiesType& )
  {
    FlexibleDataEvent ge;

    s.sends_secondary_event( ge );
    ge.set_sender( s );
    Connection< targetidentifierT >::target_.set_rport( t.handles_test_event( ge, receptor_type ) );
    Connection< targetidentifierT >::target_.set_target( &t );
  }

  /**
   * Send an event to the receiver of this connection.
   * \param e The event to send
   * \param p The port under which this connection is stored in the Connector.
   */
  bool
  send( Event& e, size_t t, const CommonSynapseProperties& )
  {
    e.set_weight( weight_ );
    e.set_receiver( *get_target( t ) );
    e.set_rport( get_rport() );
    e();
    return true;
  }

  void get_status( DictionaryDatum& d ) const;

  void set_status( const DictionaryDatum& d, ConnectorModel& cm );

  void
  set_weight( double w )
  {
    weight_ = w;
  }

  void
  set_delay( double )
  {
    throw BadProperty( "flexible_data_connection connection has no delay" );
  }

private:
  double weight_; //!< connection weight
};

template < typename targetidentifierT >
void
flexible_data_connection< targetidentifierT >::get_status( DictionaryDatum& d ) const
{
  // We have to include the delay here to prevent
  // errors due to internal calls of
  // this function in SLI/pyNEST
  ConnectionBase::get_status( d );
  def< double >( d, names::weight, weight_ );
  def< long >( d, names::size_of, sizeof( *this ) );
}

template < typename targetidentifierT >
SecondaryEvent*
flexible_data_connection< targetidentifierT >::get_secondary_event()
{
  return new FlexibleDataEvent();
}

template < typename targetidentifierT >
void
flexible_data_connection< targetidentifierT >::set_status( const DictionaryDatum& d, ConnectorModel& cm )
{
  // If the delay is set, we throw a BadProperty
  if ( d->known( names::delay ) )
  {
    throw BadProperty( "flexible_data_connection connection has no delay" );
  }

  ConnectionBase::set_status( d, cm );
  updateValue< double >( d, names::weight, weight_ );
}

} // namespace

#endif /* #ifndef FLEXIBLE_DATA_CONNECTION_H */
