/*
 *  dictionary.h
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

#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <map>
#include <string>
#include <variant>
#include <vector>

#include <boost/type_index.hpp>

#include "exceptions.h"

#include <logging.h>
#include <memory>

class dictionary_;
class dictionary;

namespace nest
{
class Parameter;
class NodeCollection;
}

typedef std::variant< size_t,
  long,
  int,
  unsigned int,
  double,
  bool,
  nest::VerbosityLevel,
  std::string,
  dictionary,
  std::shared_ptr< nest::Parameter >,
  std::shared_ptr< nest::NodeCollection >,
  std::vector< size_t >,
  std::vector< int >,
  std::vector< long >,
  std::vector< double >,
  std::vector< std::vector< long > >,
  std::vector< std::vector< double > >,
  std::vector< std::vector< std::vector< long > > >,
  std::vector< std::vector< std::vector< double > > >,
  std::vector< std::string >,
  std::vector< dictionary > >
  any_type;

class dictionary : public std::shared_ptr< dictionary_ >
{
public:
  dictionary()
    : std::shared_ptr< dictionary_ >( std::make_shared< dictionary_ >() )
  {
  }

  operator dictionary_&() const
  {
    return **this;
  }

  any_type& operator[]( const std::string& key ) const;
  any_type& operator[]( std::string&& key ) const;
  any_type& at( const std::string& key );
  const any_type& at( const std::string& key ) const;

  auto begin() const;
  auto end() const;

  auto size() const;
  auto empty() const;
  void clear() const;
  auto find( const std::string& key ) const;

  bool known( const std::string& key ) const;
  void mark_as_accessed( const std::string& key ) const;
  bool has_been_accessed( const std::string& key ) const;
  void init_access_flags( const bool thread_local_dict = false ) const;
  void
  all_entries_accessed( const std::string& where, const std::string& what, const bool thread_local_dict = false ) const;

  template < typename T >
  T get( const std::string& key ) const;

  size_t get_integer( const std::string& key ) const;

  template < typename T >
  bool update_value( const std::string& key, T& value ) const;

  template < typename T >
  bool update_integer_value( const std::string& key, T& value ) const;
};

namespace nest
{
class Parameter;
class NodeCollection;
}

/**
 * @brief Get the typename of the operand.
 *
 * @param operand to get the typename of.
 * @return std::string of the typename.
 */
std::string debug_type( const any_type& operand );

std::string debug_dict_types( const dictionary_& dict );

/**
 * @brief Check whether two any_type values are equal.
 *
 * @param first The first value.
 * @param second The other value.
 * @return Whether the values are equal, both in type and value.
 */
bool value_equal( const any_type& first, const any_type& second );

/**
 * @brief A Python-like dictionary_, based on std::map.
 *
 * Values are stored as any_type objects, with std::string keys.
 */
struct DictEntry_
{
  //! Constructor without arguments needed by std::map::operator[]
  DictEntry_()
    : item( any_type() )
    , accessed( false )
  {
  }
  DictEntry_( const any_type& item )
    : item( item )
    , accessed( false )
  {
  }

  any_type item;         //!< actual item stored
  mutable bool accessed; //!< initially false, set to true once entry is accessed
};

class dictionary_ : public std::map< std::string, DictEntry_ >
{
  // TODO-PYNEST-NG: Meta-information about entries:
  //                   * Value type (enum?)
  //                   * Whether value is writable
  //                   * Docstring for each entry
  // TODO: PYNEST-NG: maybe change to unordered map, as that provides
  // automatic hashing of keys (currently strings) which might make
  // lookups more efficient
  using maptype_ = std::map< std::string, DictEntry_ >;
  using maptype_::maptype_; // Inherit constructors

  /**
   * @brief Cast the specified non-vector value to the specified type.
   *
   * @tparam T Type of element. If the value is not of the specified type, a TypeMismatch error is thrown.
   * @param value the any object to cast.
   * @param key key where the value is located in the dictionary_, for information upon cast errors.
   * @throws TypeMismatch if the value is not of specified type T.
   * @return value cast to the specified type.
   */
  template < typename T >
  T
  cast_value_( const any_type& value, const std::string& key ) const
  {
    try
    {
      return std::get< T >( value );
    }
    catch ( const std::bad_variant_access& )
    {
      std::string msg = std::string( "Failed to cast '" ) + key + "' from " + debug_type( value ) + " to type "
        + boost::typeindex::type_id< T >().pretty_name();
      throw nest::TypeMismatch( msg );
    }
  }

  /**
   * @brief Cast the specified value to an integer.
   *
   * @param value the any object to cast.
   * @param key key where the value is located in the dictionary_, for information upon cast errors.
   * @throws TypeMismatch if the value is not an integer.
   * @return value cast to an integer.
   */
  size_t // TODO: or template?
  cast_to_integer_( const any_type& value, const std::string& key ) const
  {
    if ( std::holds_alternative< size_t >( value ) )
    {
      return cast_value_< size_t >( value, key );
    }
    else if ( std::holds_alternative< long >( value ) )
    {
      return cast_value_< long >( value, key );
    }
    else if ( std::holds_alternative< int >( value ) )
    {
      return cast_value_< int >( value, key );
    }
    // Not an integer type
    std::string msg =
      std::string( "Failed to cast '" ) + key + "' from " + debug_type( at( key ) ) + " to an integer type ";
    throw nest::TypeMismatch( msg );
  }

  void register_access_( const DictEntry_& entry ) const;

public:
  /**
   * @brief Get the value at key in the specified type.
   *
   * @tparam T Type of the value. If the value is not of the specified type, a TypeMismatch error is thrown.
   * @param key key where the value is located in the dictionary_.
   * @throws TypeMismatch if the value is not of specified type T.
   * @return the value at key cast to the specified type.
   */
  template < typename T >
  T
  get( const std::string& key ) const
  {
    return cast_value_< T >( at( key ), key );
  }

  /**
   * @brief Get the value at key as an integer.
   *
   * @param key key where the value is located in the dictionary_.
   * @throws TypeMismatch if the value is not an integer.
   * @return the value at key cast to the specified type.
   */
  size_t // TODO: or template?
  get_integer( const std::string& key ) const
  {
    return cast_to_integer_( at( key ), key );
  }

  /**
   * @brief Update the specified non-vector value if there exists a value at key.
   *
   * @param key key where the value may be located in the dictionary_.
   * @param value object to update if there exists a value at key.
   * @throws TypeMismatch if the value at key is not the same type as the value argument.
   * @return Whether value was updated.
   */
  template < typename T >
  bool
  update_value( const std::string& key, T& value ) const
  {
    auto it = find( key );
    if ( it != end() )
    {
      value = cast_value_< T >( it->second.item, key );
      return true;
    }
    return false;
  }

  /**
   * @brief Update the specified value if there exists an integer value at key.
   *
   * @param key key where the value may be located in the dictionary_.
   * @param value object to update if there exists a value at key.
   * @throws TypeMismatch if the value at key is not an integer.
   * @return Whether the value was updated.
   */
  template < typename T >
  bool
  update_integer_value( const std::string& key, T& value ) const
  {
    auto it = find( key );
    if ( it != end() )
    {
      value = cast_to_integer_( it->second.item, key );
      return true;
    }
    return false;
  }

  /**
   * @brief Check whether there exists a value with specified key in the dictionary_.
   *
   * @param key key where the value may be located in the dictionary_.
   * @return true if there is a value with the specified key, false if not.
   *
   * @note This does **not** mark the entry, because we sometimes need to confirm
   * that a certain key is not in a dictionary_.
   */
  bool
  known( const std::string& key ) const
  {
    // Bypass find() function to not set access flag
    return maptype_::find( key ) != end();
  }

  /**
   * @brief Mark entry with given key as accessed.
   */
  void
  mark_as_accessed( const std::string& key ) const
  {
    register_access_( maptype_::at( key ) );
  }

  /**
   * @brief Return true if entry has been marked as accessed.
   */
  bool
  has_been_accessed( const std::string& key ) const
  {
    return maptype_::at( key ).accessed;
  }

  /**
   * @brief Check whether the dictionary_ is equal to another dictionary_.
   *
   * Two dictionaries are equal only if they contain the exact same entries with the same values.
   *
   * @param other dictionary_ to check against.
   * @return true if the dictionary_ is equal to the other dictionary_, false if not.
   */
  bool operator==( const dictionary_& other ) const;

  /**
   * @brief Check whether the dictionary_ is unequal to another dictionary_.
   *
   * Two dictionaries are unequal if they do not contain the exact same entries with the same values.
   *
   * @param other dictionary_ to check against.
   * @return true if the dictionary_ is unequal to the other dictionary_, false if not.
   */
  bool
  operator!=( const dictionary_& other ) const
  {
    return not( *this == other );
  }

  /**
   * @brief Initializes or resets access flags for the current dictionary_.
   *
   * @note The method assumes that the dictionary_ was defined in global scope, whence it should
   * only be called from a serial context. If the dict is in thread-specific, pass `true` to
   * allow call in parallel context.
   */
  void init_access_flags( const bool thread_local_dict = false ) const;

  /**
   * @brief Check that all elements in the dictionary_ have been accessed.
   *
   * @param where Which function the error occurs in.
   * @param what Which parameter triggers the error.
   * @param thread_local_dict See note below.
   * @throws Unaccesseddictionary_Entry if there are unaccessed dictionary_ entries.
   *
   * @note The method assumes that the dictionary_ was defined in global scope, whence it should
   * only be called from a serial context. If the dict is in thread-specific, pass `true` to
   * allow call in parallel context.
   */
  void
  all_entries_accessed( const std::string& where, const std::string& what, const bool thread_local_dict = false ) const;

  // Wrappers for access flags
  any_type& operator[]( const std::string& key );
  any_type& operator[]( std::string&& key );
  any_type& at( const std::string& key );
  const any_type& at( const std::string& key ) const;
  iterator find( const std::string& key );
  const_iterator find( const std::string& key ) const;
};

std::ostream& operator<<( std::ostream& os, const dictionary_& dict );

template <>
double dictionary_::cast_value_< double >( const any_type& value, const std::string& key ) const;

template <>
std::vector< double > dictionary_::cast_value_< std::vector< double > >( const any_type& value,
  const std::string& key ) const;

inline auto
dictionary::begin() const
{
  return ( **this ).begin();
}

inline auto
dictionary::end() const
{
  return ( **this ).end();
}

template <>
std::vector< double > dictionary_::cast_value_< std::vector< double > >( const any_type& value,
  const std::string& key ) const;

inline auto
dictionary::size() const
{
  return ( **this ).size();
}

inline auto
dictionary::empty() const
{
  return ( **this ).empty();
}

inline void
dictionary::clear() const
{
  ( **this ).clear();
}

inline auto
dictionary::find( const std::string& key ) const
{
  return ( **this ).find( key );
}

inline bool
dictionary::known( const std::string& key ) const
{
  return ( **this ).known( key );
}

inline void
dictionary::mark_as_accessed( const std::string& key ) const
{
  ( **this ).mark_as_accessed( key );
}

inline bool
dictionary::has_been_accessed( const std::string& key ) const
{
  return ( **this ).has_been_accessed( key );
}

inline void
dictionary::init_access_flags( const bool thread_local_dict ) const
{
  ( **this ).init_access_flags( thread_local_dict );
}

inline void
dictionary::all_entries_accessed( const std::string& where,
  const std::string& what,
  const bool thread_local_dict ) const
{
  ( **this ).all_entries_accessed( where, what, thread_local_dict );
}

template < typename T >
inline T
dictionary::get( const std::string& key ) const
{
  return ( **this ).get< T >( key );
}

inline size_t
dictionary::get_integer( const std::string& key ) const
{
  return ( **this ).get_integer( key );
}

template < typename T >
inline bool
dictionary::update_value( const std::string& key, T& value ) const
{
  return ( **this ).update_value( key, value );
}

template < typename T >
inline bool
dictionary::update_integer_value( const std::string& key, T& value ) const
{
  return ( **this ).update_integer_value( key, value );
}

#endif /* DICTIONARY_H */
