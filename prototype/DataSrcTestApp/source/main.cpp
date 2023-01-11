#include "pyxis/utility/pyxcom.h"
#include "pyxis/data/data_driver.h"

#include <iostream>

int main(int argc, char* argv[])
{

	HRESULT hr = PYXCOMInitialize();
//#define SPECIFIC_DRIVER
#define ALL_DRIVERS
#ifdef SPECIFIC_DRIVER
	CLSID id = {0x5282faab, 0xc8bf, 0x49a8, 0x8d, 0x1, 0xdb, 0xda, 0x28, 0xa9, 0x25, 0xd};
	
  

  // Instantiate a specific process
  {

        boost::intrusive_ptr<IDataDriver> spProc;

		hr = PYXCOMCreateInstance(const_cast<const IID&>(id), 0, IDataDriver::iid, (void**) &spProc);

        if (FAILED(hr))
result
        {T

              TRACE_ERROR("couldn't		 " << id);

              return E_FAIL;

        }
		int junk;
		std::string nom = spProc->getName();
		std::cout << "The Loaded driver source name is: " << nom << std::endl;
       // TRACE_INFO("instantiated " << spProc->getName());
		std::cin >> junk;
        // no release needed

  }
#endif

#ifdef ALL_DRIVERS
    // Try to enumerate all classes

      //TRACE_INFO("Enumerating all PYXCOM classes, just for fun");

      boost::intrusive_ptr<IEnumClassObject> spECO;

      hr = PYXCOMGetClassObject(CLSID_NULL, IEnumClassObject::iid, (void**) &spECO);

      if (FAILED(hr))

      {
		  std::cout << "FAILED to enumerate classes" << std::endl;

        //TRACE_INFO("couldn't enumerate class objects");

      }

 
	  //TODO: Figure out why we find the same driver more then once. 
	  //TODO: Finding two drivers but only calling get name once ?
      while (true)
      {
        boost::intrusive_ptr<IUnknown> spCO;

        if (spECO->Next(1, (IUnknown**) &spCO, 0) != S_OK)

        {
              break;
        }

        boost::intrusive_ptr<IClassInfo> spCI;

        if (SUCCEEDED(spCO->QueryInterface(IClassInfo::iid, (void**) &spCI)))

        {
			//TODO: When getting the class if more then one exists we don't goto the next one. 
             const CLSID* pclsid;
             spCI->getClass(&pclsid);
			  boost::intrusive_ptr<IDataDriver> spProc;
			  hr = PYXCOMCreateInstance(const_cast<const IID&>(*pclsid), 0, IDataDriver::iid, (void**) &spProc);
			  std::cout << "Found: " << spProc->getName() << " " << "Class ID: " << *pclsid << std::endl;


        }

      }

#endif 
	int junk = 0;
	std::cin >> junk;
	hr = PYXCOMUninitialize();
 	return 0;
}