// $Id: fair_wavefront.cpp 5188 2012-08-30 00:31:31Z dub $

/*
 Copyright (c) 2007-2012, Trustees of The Leland Stanford Junior University
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 Redistributions of source code must retain the above copyright notice, this 
 list of conditions and the following disclaimer.
 Redistributions in binary form must reproduce the above copyright notice, this
 list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

/*fair_wavefront.cpp
 *
 *A fairer wave front allocator
 *
 */
#include "booksim.hpp"
#include <iostream>

#include "fair_wavefront.hpp"
#include "random_utils.hpp"

FairWavefront::FairWavefront( Module *parent, const string& name,
			      int inputs, int outputs ) :
DenseAllocator( parent, name, inputs, outputs ),
   _square((inputs > outputs) ? inputs : outputs), _pri(0), _num_requests(0), 
   _last_in(-1), _last_out(-1)
{
}

void FairWavefront::AddRequest( int in, int out, int label, 
				int in_pri, int out_pri )
{
  DenseAllocator::AddRequest(in, out, label, in_pri, out_pri);
  _num_requests++;
  _last_in = in;
  _last_out = out;
}

void FairWavefront::Allocate( )
{

  if(_num_requests == 0)

    // bypass allocator completely if there were no requests
    return;
  
  if(_num_requests == 1) {

    // if we only had a single request, we can immediately grant it
    _inmatch[_last_in] = _last_out;
    _outmatch[_last_out] = _last_in;
    
    // next time, start at the diagonal after the one with the request in it
    _pri = ( _last_in + ( _square - _last_out ) + 1 ) % _square;
    
  } else {

    // otherwise we have to loop through the diagonals of request matrix

    int next_pri = -1;

    for ( int p = 0; p < _square; ++p ) {
      for ( int q = 0; q < _square; ++q ) {
	int input = ( ( _pri + p ) + ( _square - q ) ) % _square;
	int output = q;
	
	if ( ( input < _inputs ) && ( output < _outputs ) && 
	     ( _inmatch[input] == -1 ) && ( _outmatch[output] == -1 ) &&
	     ( _request[input][output].label != -1 ) ) {
	  // Grant!
	  _inmatch[input] = output;
	  _outmatch[output] = input;

	  // if this is the first diagonal to have any requests, start at the 
	  // next one the next time around
	  if(next_pri < 0) {
	    next_pri = ((_pri + p) + 1) % _square;
	  }

	}
      }
    }
    
    // update priority diagonal
    _pri = next_pri;

  }
  
  _num_requests = 0;
  _last_in = -1;
  _last_out = -1;
}


