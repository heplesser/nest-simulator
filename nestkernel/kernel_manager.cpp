/*
 *  kernel_manager.cpp
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

#include "kernel_manager.h"

nest::KernelManager* nest::KernelManager::kernel_manager_instance_ = nullptr;


dictionary
nest::KernelManager::get_build_info_()
{
  // Exit codes
  constexpr unsigned int EXITCODE_UNKNOWN_ERROR = 10;
  constexpr unsigned int EXITCODE_USERABORT = 15;
  constexpr unsigned int EXITCODE_EXCEPTION = 125;
  constexpr unsigned int EXITCODE_SCRIPTERROR = 126;
  constexpr unsigned int EXITCODE_FATAL = 127;

  // The range 200-215 is reserved for test skipping exitcodes. Any new codes must
  // also be added to testsuite/do_tests_sh.in.
  constexpr unsigned int EXITCODE_SKIPPED = 200;
  constexpr unsigned int EXITCODE_SKIPPED_NO_MPI = 201;
  constexpr unsigned int EXITCODE_SKIPPED_HAVE_MPI = 202;
  constexpr unsigned int EXITCODE_SKIPPED_NO_THREADING = 203;
  constexpr unsigned int EXITCODE_SKIPPED_NO_GSL = 204;
  constexpr unsigned int EXITCODE_SKIPPED_NO_MUSIC = 205;

  dictionary build_info;

  build_info[ "version" ] = std::string( NEST_VERSION );
  build_info[ "exitcode" ] = EXIT_SUCCESS;
  build_info[ "built" ] = std::string( String::compose( "%1 %2", __DATE__, __TIME__ ) );
  build_info[ "data_dir" ] = std::string( NEST_INSTALL_PREFIX "/" NEST_INSTALL_DATADIR );
  build_info[ "doc_dir" ] = std::string( NEST_INSTALL_PREFIX "/" NEST_INSTALL_DOCDIR );
  build_info[ "prefix" ] = std::string( NEST_INSTALL_PREFIX );
  build_info[ "host" ] = std::string( NEST_HOST );
  build_info[ "hostos" ] = std::string( NEST_HOSTOS );
  build_info[ "hostvendor" ] = std::string( NEST_HOSTVENDOR );
  build_info[ "hostcpu" ] = std::string( NEST_HOSTCPU );

#ifdef _OPENMP
  build_info[ "have_threads" ] = true;
  build_info[ "threads_model" ] = std::string( "openmp" );
#else
  build_info[ "have_threads" ] = false;
#endif

#ifdef HAVE_MPI
  build_info[ "have_mpi" ] = true;
  build_info[ "mpiexec" ] = std::string( MPIEXEC );
  build_info[ "mpiexec_numproc_flag" ] = std::string( MPIEXEC_NUMPROC_FLAG );
  build_info[ "mpiexec_max_numprocs" ] = std::string( MPIEXEC_MAX_NUMPROCS );
  build_info[ "mpiexec_preflags" ] = std::string( MPIEXEC_PREFLAGS );
  build_info[ "mpiexec_postflags" ] = std::string( MPIEXEC_POSTFLAGS );
#else
  build_info[ "have_mpi" ] = false;
#endif

#ifdef HAVE_GSL
  build_info[ "have_gsl" ] = true;
#else
  build_info[ "have_gsl" ] = false;
#endif

#ifdef HAVE_MUSIC
  build_info[ "have_music" ] = true;
#else
  build_info[ "have_music" ] = false;
#endif

#ifdef HAVE_LIBNEUROSIM
  build_info[ "have_libneurosim" ] = true;
#else
  build_info[ "have_libneurosim" ] = false;
#endif

#ifdef HAVE_SIONLIB
  build_info[ "have_sionlib" ] = true;
#else
  build_info[ "have_sionlib" ] = false;
#endif

#ifdef NDEBUG
  build_info[ "ndebug" ] = true;
#else
  build_info[ "ndebug" ] = false;
#endif

  dictionary exitcodes;

  exitcodes[ "success" ] = EXIT_SUCCESS;
  exitcodes[ "skipped" ] = EXITCODE_SKIPPED;
  exitcodes[ "skipped_no_mpi" ] = EXITCODE_SKIPPED_NO_MPI;
  exitcodes[ "skipped_have_mpi" ] = EXITCODE_SKIPPED_HAVE_MPI;
  exitcodes[ "skipped_no_threading" ] = EXITCODE_SKIPPED_NO_THREADING;
  exitcodes[ "skipped_no_gsl" ] = EXITCODE_SKIPPED_NO_GSL;
  exitcodes[ "skipped_no_music" ] = EXITCODE_SKIPPED_NO_MUSIC;
  exitcodes[ "scripterror" ] = EXITCODE_SCRIPTERROR;
  exitcodes[ "abort" ] = NEST_EXITCODE_ABORT;
  exitcodes[ "userabort" ] = EXITCODE_USERABORT;
  exitcodes[ "segfault" ] = NEST_EXITCODE_SEGFAULT;
  exitcodes[ "exception" ] = EXITCODE_EXCEPTION;
  exitcodes[ "fatal" ] = EXITCODE_FATAL;
  exitcodes[ "unknownerror" ] = EXITCODE_UNKNOWN_ERROR;

  build_info[ "test_exitcodes" ] = exitcodes;

  return build_info;
}

void
nest::KernelManager::create_kernel_manager()
{
#pragma omp critical( create_kernel_manager )
  {
    if ( not kernel_manager_instance_ )
    {
      kernel_manager_instance_ = new KernelManager();
      assert( kernel_manager_instance_ );
    }
  }
}

void
nest::KernelManager::destroy_kernel_manager()
{
  kernel_manager_instance_->logging_manager.set_logging_level( M_QUIET );
  delete kernel_manager_instance_;
}

nest::KernelManager::KernelManager()
  : fingerprint_( 0 )
  , logging_manager()
  , mpi_manager()
  , vp_manager()
  , random_manager()
  , simulation_manager()
  , modelrange_manager()
  , connection_manager()
  , sp_manager()
  , event_delivery_manager()
  , model_manager()
  , music_manager()
  , node_manager()
  , io_manager()
  , managers( { &logging_manager,
      &mpi_manager,
      &vp_manager,
      &random_manager,
      &simulation_manager,
      &modelrange_manager,
      &model_manager,
      &connection_manager,
      &sp_manager,
      &event_delivery_manager,
      &music_manager,
      &io_manager,
      &node_manager } )
  , initialized_( false )
{
}

nest::KernelManager::~KernelManager()
{
}

void
nest::KernelManager::initialize()
{
  for ( auto& manager : managers )
  {
    manager->initialize();
  }

  ++fingerprint_;
  initialized_ = true;
}

void
nest::KernelManager::prepare()
{
  for ( auto& manager : managers )
  {
    manager->prepare();
  }
}

void
nest::KernelManager::cleanup()
{
  for ( auto&& m_it = managers.rbegin(); m_it != managers.rend(); ++m_it )
  {
    ( *m_it )->cleanup();
  }
}

void
nest::KernelManager::finalize()
{
  for ( auto&& m_it = managers.rbegin(); m_it != managers.rend(); ++m_it )
  {
    ( *m_it )->finalize();
  }
  initialized_ = false;
}

void
nest::KernelManager::reset()
{
  finalize();
  initialize();
}

void
nest::KernelManager::change_number_of_threads( size_t new_num_threads )
{
  // Inputs are checked in VPManager::set_status().
  // Just double check here that all values are legal.
  assert( node_manager.size() == 0 );
  assert( not connection_manager.get_user_set_delay_extrema() );
  assert( not simulation_manager.has_been_simulated() );
  assert( not sp_manager.is_structural_plasticity_enabled() or new_num_threads == 1 );

  vp_manager.set_num_threads( new_num_threads );
  for ( auto& manager : managers )
  {
    manager->change_number_of_threads();
  }
}

void
nest::KernelManager::set_status( const dictionary& dict )
{
  assert( is_initialized() );

  for ( auto& manager : managers )
  {
    manager->set_status( dict );
  }
}

void
nest::KernelManager::get_status( dictionary& dict )
{
  assert( is_initialized() );

  for ( auto& manager : managers )
  {
    manager->get_status( dict );
  }

  dict[ "build_info" ] = get_build_info_();
}
