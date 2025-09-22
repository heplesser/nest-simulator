/*
 *  main.cpp
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

#include "nest.h"
#include <cstdlib>
#include <iomanip>
#include <iostream>

// Cut and pasted here to get around privatness in KernelManager

#ifdef __linux__

#include <fstream>
#include <sstream>
size_t
get_memsize_linux_()
{
  // code based on mistral.ai
  std::ifstream file( "/proc/self/status" );
  if ( not file.is_open() )
  {
    throw std::runtime_error( "Could not open /proc/self/status" );
  }

  std::string line;
  while ( std::getline( file, line ) )
  {
    if ( line.rfind( "VmSize:", 0 ) == 0 )
    {
      std::istringstream stream( line );
      std::string key;
      size_t value;
      std::string unit;
      stream >> key >> value >> unit;
      file.close();
      if ( unit != "kB" )
      {
        throw std::runtime_error( "VmSize not reported in kB" );
      }
      return value;
    }
  }

  file.close();
  throw std::runtime_error( "VmSize not found in /proc/self/status" );
}

#else

size_t
get_memsize_linux_()
{
  assert( false || "Only implemented on Linux systems." );
  return 0;
}

#endif


#if defined __APPLE__

#include <mach/mach.h>
size_t
get_memsize_darwin_()
{
  struct task_basic_info t_info;
  mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;

  kern_return_t result = task_info( mach_task_self(), TASK_BASIC_INFO, ( task_info_t ) &t_info, &t_info_count );
  assert( result == KERN_SUCCESS || "Problem occurred during getting of task_info." );

  // For macOS, vmsize is not informative, it is an extremly large address range, usually O(2^40).
  // resident_size gives the most reasonable information. Information is in bytes, thus divide.
  return t_info.resident_size / 1024;
}

#else

size_t
get_memsize_darwin_()
{
  assert( false || "Only implemented on macOS." );
  return 0;
}

#endif

// End cut-and-paster

size_t
get_memsize()
{
  if ( NEST_HOSTOS == std::string( "linux" ) )
  {
    return get_memsize_linux_();
  }
  else if ( NEST_HOSTOS == std::string( "darwin" ) )
  {
    return get_memsize_darwin_();
  }
  else
  {
    assert( false );
  }
}


int
main( int argc, char* argv[] )
{
  assert( argc == 4 );
  const int thr_req = std::atoi( argv[ 1 ] );
  const int n = std::atoi( argv[ 2 ] );
  const int indegree = std::atoi( argv[ 3 ] );
  std::cerr << std::fixed << std::setprecision( 3 );

  std::cerr << "Memory at start     : " << std::setw( 10 ) << get_memsize() / 1024. << " MB" << std::endl;

  nest::init_nest( &argc, &argv );

  std::cerr << "Memory after init   : " << std::setw( 10 ) << get_memsize() / 1024. << " MB" << std::endl;

  dictionary nd;
  nd[ "local_num_threads" ] = thr_req;
  nest::set_kernel_status( nd );

  std::cerr << "Memory after threads: " << std::setw( 10 ) << get_memsize() / 1024. << " MB" << std::endl;

  auto nc = nest::create( "iaf_psc_alpha", n );

  std::cerr << "Memory after Create : " << std::setw( 10 ) << get_memsize() / 1024. << " MB" << std::endl;

  dictionary conn_spec;
  conn_spec[ "rule" ] = std::string( "fixed_indegree" );
  conn_spec[ "indegree" ] = static_cast< long >( indegree );
  std::vector< dictionary > syn_spec;
  dictionary syn_spec_one;
  syn_spec_one[ "synapse_model" ] = std::string( "static_synapse" );
  syn_spec.push_back( syn_spec_one );
  nest::connect( nc, nc, conn_spec, syn_spec );

  std::cerr << "Memory after Connect: " << std::setw( 10 ) << get_memsize() / 1024. << " MB" << std::endl;

  return 0;
}
