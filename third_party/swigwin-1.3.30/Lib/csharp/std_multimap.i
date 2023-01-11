/* -----------------------------------------------------------------------------
 * This file did not come with SWIG.  It has been created based on the SWIG 
 * std_map.i file.  The multimap insert() method has not yet been implemented 
 * in this file, therefore, new entries to a multimap cannot be made in C#.
 * See the LICENSE file for information on copyright, usage and redistribution
 * of SWIG, and the README file for authors - http://www.swig.org/release.html.
 *
 * std_multimap.i
 *
 * SWIG typemaps for std::multimap
 * ----------------------------------------------------------------------------- */

%include <std_common.i>

// ------------------------------------------------------------------------
// std::multimap
// ------------------------------------------------------------------------

%{
#include <map>
#include <algorithm>
#include <stdexcept>
%}

// exported class

namespace std {

    template<class K, class T> class multimap {
        // add typemaps here
      public:
        multimap();
        multimap(const multimap<K,T> &);
        
        unsigned int size() const;
        bool empty() const;
        void clear();
        %extend {
            T& get(const K& key) throw (std::out_of_range) {
                std::multimap<K,T >::iterator i = self->find(key);
                if (i != self->end())
                    return i->second;
                else
                    throw std::out_of_range("key not found");
            }
            void del(const K& key) throw (std::out_of_range) {
                std::multimap<K,T >::iterator i = self->find(key);
                if (i != self->end())
                    self->erase(i);
                else
                    throw std::out_of_range("key not found");
            }
            bool has_key(const K& key) {
                std::multimap<K,T >::iterator i = self->find(key);
                return i != self->end();
            }
        }
    };


    // specializations for built-ins

    // add specializations here

}
