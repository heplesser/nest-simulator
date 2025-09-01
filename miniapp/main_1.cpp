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
#include <iostream>

int
main( int argc, char* argv[] )
{
  assert( argc == 2 );
  const int thr_req = std::atoi( argv[ 1 ] );

  std::cerr << "Requested number of threads: " << thr_req << std::endl;

  nest::init_nest( &argc, &argv );

  dictionary d = nest::get_kernel_status();
  std::cerr << "Threads after startup: " << d.get_integer( "local_num_threads" ) << std::endl;

  std::cerr << "Attempt to set new number of threads: " << thr_req << std::endl;
  dictionary nd;
  nd[ "local_num_threads" ] = thr_req;
  nest::set_kernel_status( nd );

  dictionary d2 = nest::get_kernel_status();
  std::cerr << "Threads after change: " << d2.get_integer( "local_num_threads" ) << std::endl;

  return 0;
}
