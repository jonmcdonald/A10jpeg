/* -*-mode:c++-*- *********************************************************
 *                                                                        *
 *  Algorithmic C (tm) Datatypes                                          *
 *                                                                        *
 *  Software Version: 3.4                                                 *
 *                                                                        *
 *  Release Date    : Wed Jul 22 21:51:58 PDT 2015                        *
 *  Release Type    : Production Release                                  *
 *  Release Build   : 3.4.0                                               *
 *                                                                        *
 *  Copyright 2004-2014, Calypto Design Systems, Inc.,                     *
 *                                                                        *
 *  All Rights Reserved.                                                  *
 *                                                                        *
 **************************************************************************
 *                                                                        *
 *  The most recent version of this package can be downloaded from:       *
 *     http://www.calypto.com/ac_datatypes.php                            *
 *                                                                        *
 **************************************************************************
 *                                                                        *
 *  IMPORTANT - THIS SOFTWARE IS COPYRIGHTED AND SUBJECT TO LICENSE       *
 *  RESTRICTIONS                                                          *
 *                                                                        *
 *  THE LICENSE THAT CONTROLS YOUR USE OF THE SOFTWARE IS:                *
 *     ALGORITHMIC C DATATYPES END-USER LICENSE AGREEMENT                 *
 *                                                                        *
 *  THESE COMMENTS ARE NOT THE LICENSE.  PLEASE CONSULT THE FULL LICENSE  *
 *  FOR THE ACTUAL TERMS AND CONDITIONS WHICH IS LOCATED AT THE BOTTOM    *
 *  OF THIS FILE.                                                         *
 *                                                                        *  
 *  CAREFULLY READ THE LICENSE AGREEMENT BEFORE USING THE SOFTWARE.       *
 *                                                                        *  
 *       *** MODIFICATION OF THE SOFTWARE IS NOT AUTHORIZED ***           *
 *                                                                        *
 **************************************************************************
 *                                                                        *
 *  YOUR USE OF THE SOFTWARE INDICATES YOUR COMPLETE AND UNCONDITIONAL    *
 *  ACCEPTANCE OF THE TERMS AND CONDITIONS SET FORTH IN THE LICENSE. IF   *
 *  YOU DO NOT  AGREE TO THE LICENSE TERMS AND CONDITIONS, DO NOT USE THE *
 *  SOFTWARE, REMOVE IT FROM YOUR SYSTEM, AND DESTROY ALL COPIES.         *
 *                                                                        *
 *************************************************************************/

/*
//  Source:         ac_channel.h
//  Description:    templatized channel communication class
//  Author:         Andres Takach, Ph.D.
*/

#ifndef __AC_CHANNEL_H
#define __AC_CHANNEL_H

#include <iostream>
#include <stdio.h>

#ifndef __SYNTHESIS__
#include <deque>
#include <string>
#include <algorithm>
#endif

// Macro Definitions (obsolete - provided here for backward compatibility)
#define AC_CHAN_CTOR(varname) varname
#define AC_CHAN_CTOR_INIT(varname,init) varname(init)
#define AC_CHAN_CTOR_VAL(varname,init,val) varname(init,val)

///////////////////////////////////////////
// Class: ac_channel
//////////////////////////////////////////

template <class T>
class  ac_channel
{
public:

	T read();
	void read(T& t) ;
	void write(const T& t);

	// constructors
	ac_channel();  
	ac_channel(int init);
	ac_channel(int init, T val);

	bool available(unsigned int k);  // Return true if channel has at least k entries
	unsigned int  size();
	bool empty() {return size() == 0;}
 
	unsigned int debug_size() { 
#ifndef __SYNTHESIS__
      return chan.size(); 
#endif
   }

	T    operator[](unsigned int pos) const {
#ifndef __SYNTHESIS__
      return chan[pos];
#endif
   }

#ifndef __SYNTHESIS__
	// These functions are useful for writing the testbench and the 
	// scverify flow but are not supported for synthesis.  
	bool operator==(const ac_channel &rhs) const { return this->chan == rhs.chan; }
	bool operator!=(const ac_channel &rhs) const { return !operator==(rhs); }
	void  reset() {chan.clear(); for (int i=0; i<(int)rSz; i++) write(rVal);}
 
	bool nb_read(T& t)   {
		if (size() != 0) {
			  read(t);
			  return true;
		} else return false;
	}
  
	int get_size_call_count() { int tmp=size_call_count; size_call_count=0; return tmp; }

	// data - The verifcation flow needs it to be public
	std::deque<T> chan;
#else
	// for synthesis
	bool nb_read(T& t);

private:
	// Prevent the compiler from autogenerating these.  (This enforces that channels are always passed by
	// reference.)  
	ac_channel(const ac_channel< T >&); 
	ac_channel& operator=(const ac_channel< T >&);
#endif

private:
 // data
#ifndef __SYNTHESIS__
	unsigned int rSz;    // reset size
	T            rVal;   // resetValue
	int          size_call_count;
#else 
   T chan;  
#endif
};

// constructors with no name ID

template <class T>
ac_channel<T>::ac_channel()
#ifndef __SYNTHESIS__
	: rSz(0)
	, size_call_count(0)
#endif
{}

// Default for types we don't know about
template <class T>
ac_channel<T>::ac_channel(int init) 
#ifndef __SYNTHESIS__
	: rSz(init)
	, size_call_count(0)
#endif
{
	for (int i=init; i> 0; i--) {
		T dc; 
		write(dc);
	}
}

template <class T>
ac_channel<T>::ac_channel(int init, T val) 
#ifndef __SYNTHESIS__
	: rSz(init)
	, rVal(val)
	, size_call_count(0)
#endif
{
	for (int i=init; i> 0; i--)
		write(val);
}

//The actual hardware looks very much like the SYNTHESIS model.  
// The 2 and 3 argument CTORs store the size arg
// in an integer variable similar to rSz.  The allows us to use a FIFO length which is 
// independent of the startup time of the design.

template <class T>
T ac_channel<T>::read()
{
#ifndef __SYNTHESIS__
	if (chan.empty()) {
          std::cout << "Read from empty channel" << std::endl;  
          throw("Read from empty channel");  
        }
	T t= chan.front(); 
	chan.pop_front(); 
	return t;
#else
	// this is non-sense (doesn't get used, right?)
  T dc;
  unsigned int chan_rSz = 0;
  if (chan_rSz == 0) {
    return chan;
  } else {
    chan_rSz--;
    return dc;
  }
#endif
}

template <class T>
void ac_channel<T>::read(T& t) 
{
	t = read();
}

template <class T>
void ac_channel<T>::write(const T& t) 
{
#ifndef __SYNTHESIS__
	chan.push_back(t);
#endif
}

template <class T>
unsigned int  ac_channel<T>::size()
{
#ifndef __SYNTHESIS__
	size_call_count++;
	return (int)chan.size();
#endif
}

template <class T>
bool ac_channel<T>::available(unsigned int k)
{ 
#ifndef __SYNTHESIS__
	return chan.size() >= k;
#endif
}

template<class T>
inline std::ostream& operator<< (std::ostream& os, ac_channel<T> &a)
{
#ifndef __SYNTHESIS__
  for (unsigned int i=0; i<a.size(); i++) {
    if (i > 0) os << " ";
    os << a[i];
  }
#endif
  return os;
}

#endif


/**************************************************************************
 *                                                                        *
 *  ALGORITHMIC C DATATYPES END-USER LICENSE AGREEMENT                    *
 *                                                                        *
 *                                                                        *
 *  IMPORTANT - USE OF SOFTWARE IS SUBJECT TO LICENSE RESTRICTIONS        *
 *  CAREFULLY READ THIS LICENSE AGREEMENT BEFORE USING THE SOFTWARE       *
 *                                                                        *
 *  YOU MAY USE AND DISTRIBUTE UNMODIFIED VERSIONS OF THIS SOFTWARE AS    *
 *  STATED BELOW, YOU MAY NOT MODIFY THE SOFTWARE This license is a       *
 *  legal Agreement between you, the end user, either individually or     *
 *  as an authorized representative of a company acquiring the license,   *
 *  and Calypto Design Systems, Inc. ("Calypto"). YOUR USE OF             *
 *  THE SOFTWARE INDICATES YOUR COMPLETE AND UNCONDITIONAL ACCEPTANCE     *
 *  OF THE TERMS AND CONDITIONS SET FORTH IN THIS AGREEMENT. If you do    *
 *  not agree to these terms and conditions, promptly return or, if       *
 *  received electronically, delete the Software and all accompanying     *
 *  items.                                                                *
 *                                                                        *
 *                                                                        *
 *  1. GRANT OF LICENSE. YOU MAY USE AND DISTRIBUTE THE SOFTWARE, BUT     *
 *  YOU MAY NOT MODIFY THE SOFTWARE. The Software you are installing,     *
 *  downloading, or otherwise acquired, under this Agreement, including   *
 *  source code, binary code, updates, modifications, revisions,          *
 *  copies, or documentation pertaining to Algorithmic C Datatypes        *
 *  (collectively the "Software") is a copyrighted work owned by          *
 *  Calypto. Calypto grants to you, a nontransferable,                    *
 *  nonexclusive, limited copyright license to use and distribute the     *
 *  Software, but you may not modify the Software. Use of the Software    *
 *  consists solely of reproduction, performance, and display.            *
 *                                                                        *
 *  2. RESTRICTIONS; NO MODIFICATION. Modifying the Software is           *
 *  prohibited. Each copy of the Software you create must include all     *
 *  notices and legends embedded in the Software.  Modifying the          *
 *  Software means altering, enhancing, editing, deleting portions or     *
 *  creating derivative works of the Software.  You may append other      *
 *  code to the Software, so long as the Software is not otherwise        *
 *  modified. Calypto retains all rights not expressly granted            *
 *  by this Agreement. The terms of this Agreement, including without     *
 *  limitation, the licensing and assignment provisions, shall be         *
 *  binding upon your successors in interest and assigns.  The            *
 *  provisions of this section 2 shall survive termination or             *
 *  expiration of this Agreement.                                         *
 *                                                                        *
 *  3. USER COMMENT AND SUGGESTIONS.  You are not obligated to provide    *
 *  Calypto with comments or suggestions regarding the                    *
 *  Software.  However, if you do provide to Calypto comments             *
 *  or suggestions for the modification, correction, improvement or       *
 *  enhancement of (a) the Software or (b) Calypto products or            *
 *  processes which may embody the Software ("Comments"), you grant to    *
 *  Calypto a non-exclusive, irrevocable, worldwide, royalty-free         *
 *  license to disclose, display, perform, copy, make, have made, use,    *
 *  sublicense, sell, and otherwise dispose of the Comments, and          *
 *  Calypto's products embodying such Comments, in any manner which       *
 *  Calypto chooses, without reference to the source.                     *
 *                                                                        *
 *  4. NO WARRANTY. CALYPTO EXPRESSLY DISCLAIMS ALL WARRANTY              *
 *  FOR THE SOFTWARE. TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE       *
 *  LAW, THE SOFTWARE AND ANY RELATED DOCUMENTATION IS PROVIDED "AS IS"   *
 *  AND WITH ALL FAULTS AND WITHOUT WARRANTIES OR CONDITIONS OF ANY       *
 *  KIND, EITHER EXPRESS OR IMPLIED, INCLUDING, WITHOUT LIMITATION, THE   *
 *  IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR       *
 *  PURPOSE, OR NONINFRINGEMENT. THE ENTIRE RISK ARISING OUT OF USE OR    *
 *  DISTRIBUTION OF THE SOFTWARE REMAINS WITH YOU.                        *
 *                                                                        *
 *  5. LIMITATION OF LIABILITY. IN NO EVENT WILL CALYPTO OR ITS           *
 *  LICENSORS BE LIABLE FOR INDIRECT, SPECIAL, INCIDENTAL, OR             *
 *  CONSEQUENTIAL DAMAGES (INCLUDING LOST PROFITS OR SAVINGS) WHETHER     *
 *  BASED ON CONTRACT, TORT OR ANY OTHER LEGAL THEORY, EVEN IF CALYPTO    *
 *  OR ITS LICENSORS HAVE BEEN ADVISED OF THE POSSIBILITY OF              *
 *  SUCH DAMAGES.                                                         *
 *                                                                        *
 *  6.  LIFE ENDANGERING APPLICATIONS. NEITHER CALYPTO NOR ITS            *
 *  LICENSORS SHALL BE LIABLE FOR ANY DAMAGES RESULTING FROM OR IN        *
 *  CONNECTION WITH THE USE OR DISTRIBUTION OF SOFTWARE IN ANY            *
 *  APPLICATION WHERE THE FAILURE OR INACCURACY OF THE SOFTWARE MIGHT     *
 *  RESULT IN DEATH OR PERSONAL INJURY.  THE PROVISIONS OF THIS SECTION 6 *
 *  SHALL SURVIVE TERMINATION OR EXPIRATION OF THIS AGREEMENT.            *
 *                                                                        *
 *  7.  INDEMNIFICATION.  YOU AGREE TO INDEMNIFY AND HOLD HARMLESS        *
 *  CALYPTO AND ITS LICENSORS FROM ANY CLAIMS, LOSS, COST,                *
 *  DAMAGE, EXPENSE, OR LIABILITY, INCLUDING ATTORNEYS' FEES, ARISING     *
 *  OUT OF OR IN CONNECTION WITH YOUR USE OR DISTRIBUTION OF SOFTWARE.    *
 *                                                                        *
 *  8. TERM AND TERMINATION. This Agreement terminates immediately if     *
 *  you exceed the scope of the license granted or fail to comply with    *
 *  the provisions of this License Agreement.  If you institute patent    *
 *  litigation against Calypto (including a cross-claim or                *
 *  counterclaim in a lawsuit) alleging that the Software constitutes     *
 *  direct or contributory patent infringement, then any patent           *
 *  licenses granted to you under this License for that Software shall    *
 *  terminate as of the date such litigation is filed. Upon termination   *
 *  or expiration, you agree to cease all use of the Software and         *
 *  delete all copies of the Software.                                    *
 *                                                                        *
 *  9. EXPORT. Software may be subject to regulation by local laws and    *
 *  United States government agencies, which prohibit export or           *
 *  diversion of certain products, information about the products, and    *
 *  direct products of the products to certain countries and certain      *
 *  persons. You agree that you will not export any Software or direct    *
 *  product of Software in any manner without first obtaining all         *
 *  necessary approval from appropriate local and United States           *
 *  government agencies.                                                  *
 *                                                                        *
 *  10. U.S. GOVERNMENT LICENSE RIGHTS. Software was developed entirely   *
 *  at private expense. All software is commercial computer software      *
 *  within the meaning of the applicable acquisition regulations.         *
 *  Accordingly, pursuant to US FAR 48 CFR 12.212 and DFAR 48 CFR         *
 *  227.7202, use, duplication and disclosure of the Software by or for   *
 *  the U.S. Government or a U.S. Government subcontractor is subject     *
 *  solely to the terms and conditions set forth in this Agreement,       *
 *  except for provisions which are contrary to applicable mandatory      *
 *  federal laws.                                                         *
 *                                                                        *
 *  11. CONTROLLING LAW AND JURISDICTION. THIS AGREEMENT SHALL BE         *
 *  GOVERNED BY AND CONSTRUED UNDER THE LAWS OF THE STATE OF CALIFORNIA,  *
 *  USA. All disputes arising out of or in relation to this Agreement     *
 *  shall be submitted to the exclusive jurisdiction of Santa Clara       *
 *  County, California. This section shall not restrict Calypto's         *
 *  right to bring an action against you in the jurisdiction where your   *
 *  place of business is located.  The United Nations Convention on       *
 *  Contracts for the International Sale of Goods does not apply to       *
 *  this Agreement.                                                       *
 *                                                                        *
 *  12. SEVERABILITY. If any provision of this Agreement is held by a     *
 *  court of competent jurisdiction to be void, invalid, unenforceable    *
 *  or illegal, such provision shall be severed from this Agreement and   *
 *  the remaining provisions will remain in full force and effect.        *
 *                                                                        *
 *  13. MISCELLANEOUS.  This Agreement contains the parties' entire       *
 *  understanding relating to its subject matter and supersedes all       *
 *  prior or contemporaneous agreements. This Agreement may only be       *
 *  modified in writing by authorized representatives of the parties.     *
 *  Waiver of terms or excuse of breach must be in writing and shall      *
 *  not constitute subsequent consent, waiver or excuse. The prevailing   *
 *  party in any legal action regarding the subject matter of this        *
 *  Agreement shall be entitled to recover, in addition to other          *
 *  relief, reasonable attorneys' fees and expenses.                      *
 *                                                                        *
 *  Algorithmic C Datatypes EULA (Rev. 110928)                            *
 **************************************************************************/
