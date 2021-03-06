// Device.h -*- C++ -*-

//
// Copyright (c) 2012 Kimball Thurston
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
// OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#pragma once

#include <cstdint>

#include "libusb-1.0/libusb.h"
#include <functional>
#include "Exception.h"
#include "Transfer.h"
#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <string>


////////////////////////////////////////


///
/// @file Device.h
///
/// @author Kimball Thurston
///

namespace USB
{

///
/// @brief Class Device provides...
///
class Device
{
public:
	Device( void );
	Device( libusb_device *dev );
	Device( libusb_device *dev, const struct libusb_device_descriptor &desc );
	virtual ~Device( void );

	Device( const Device &o ) = delete;
	Device &operator=( const Device &o ) = delete;

	void setContext( libusb_context *ctxt ) { myContext = ctxt; }

	void setDevice( libusb_device *dev );
	void setDevice( libusb_device *dev, const struct libusb_device_descriptor &desc );
	inline libusb_device *getDevice( void ) const { return myDevice; }

	bool isValid( void ) const { return myDevice != nullptr; }

	const std::string &getProductName( void ) const { return myProduct; }
	const std::string &getManufacturer( void ) const { return myManufacturer; }
	const std::string &getSerialNumber( void ) const { return mySerialNumber; }

	const struct libusb_device_descriptor &desc( void ) const { return myDescriptor; }

	inline bool matches( uint16_t v ) const { return myDescriptor.idVendor == v; }
	inline bool matches( uint16_t v, uint16_t p ) const { return myDescriptor.idVendor == v && myDescriptor.idProduct == p; }

	void claimInterfaces( void );

	void startEventHandling( void );
	void stopEventHandling( void );
	void shutdown( void );

	void setLanguageID( uint16_t langID = 0 );

	void dumpInfo( std::ostream &os );

	void dispatchEvent( libusb_transfer *xfer );

	static constexpr uint8_t endpoint_in( uint8_t i ) { return LIBUSB_ENDPOINT_IN | (LIBUSB_ENDPOINT_ADDRESS_MASK & i); }
	static constexpr uint8_t endpoint_out( uint8_t i ) { return LIBUSB_ENDPOINT_OUT | (LIBUSB_ENDPOINT_ADDRESS_MASK & i); }

protected:
	// returns true to requeue transfer (i.e. for input interrupt transfers)
	virtual bool handleEvent( int endpoint, uint8_t *buf, int buflen, libusb_transfer *xfer ) = 0;

	virtual bool wantInterface( const struct libusb_interface_descriptor &iface ) = 0;

	virtual std::shared_ptr<AsyncTransfer> createTransfer( const struct libusb_endpoint_descriptor &epDesc );

	virtual void parseInterface( uint8_t inum, const struct libusb_interface_descriptor &ifacedesc );

	virtual void dumpExtraInterfaceInfo( std::ostream &os, const struct libusb_interface_descriptor &iface );

	void openHandle( void );
	virtual void closeHandle( void );

	std::string pullString( uint8_t desc_idx );

	libusb_context *myContext = nullptr;
	libusb_device *myDevice = nullptr;
	libusb_device_handle *myHandle = nullptr;
	struct libusb_device_descriptor myDescriptor;
	std::vector<libusb_config_descriptor *> myConfigs;
	libusb_bos_descriptor *myBOS = nullptr;
	libusb_usb_2_0_extension_descriptor *myUSB2Ext = nullptr;
	libusb_ss_usb_device_capability_descriptor *mySSDesc = nullptr;
	std::map<uint8_t, libusb_ss_endpoint_companion_descriptor *> mySSEndpointCompanion;

	int mySpeed = 0;

	size_t myCurConfig = 0;
	std::vector<uint8_t> myClaimedInterfaces;
	std::vector< std::shared_ptr<AsyncTransfer> > myInputs;
	std::string myManufacturer;
	std::string myProduct;
	std::string mySerialNumber;
	uint16_t myLangID = 0;
};

} // namespace USB

