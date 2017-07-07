///////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2014-2015 libbitcoin-database developers (see COPYING).
//
//        GENERATED SOURCE CODE, DO NOT EDIT EXCEPT EXPERIMENTALLY
//
///////////////////////////////////////////////////////////////////////////////
#ifndef MVS_DATABASE_VERSION_HPP
#define MVS_DATABASE_VERSION_HPP

/**
 * The semantic version of this repository as: [major].[minor].[patch]
 * For interpretation of the versioning scheme see: http://semver.org
 */
/*           chenhao     init value  0.6.0
 * 2017.6.11 wangdongyun modify from 0.6.0 to 0.6.1 
 * 1. replace asset with blockchain_asset, to contain asset blockchain height 
 * 	  to resolve block not sync when same asset exist
 *
 * 2017.7.7 wangdongyun modify to 0.6.2
 * 1. modification in 0.6.1 must let user to resync block data from height 1. this will waste too long time.
 *    this version is enhanced to read block data from local block database not resysn block data from p2p network. 
 */
#define MVS_DATABASE_VERSION "0.6.2"

#define MVS_DATABASE_MAJOR_VERSION 0
#define MVS_DATABASE_MINOR_VERSION 6
#define MVS_DATABASE_PATCH_VERSION 2

#endif
