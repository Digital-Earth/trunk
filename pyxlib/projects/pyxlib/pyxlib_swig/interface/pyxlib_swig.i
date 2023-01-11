
%module (directors="1") pyxlib

// Suppress warning: Covariant return types not supported in C#
#pragma SWIG nowarn=842

// Standard type mappings
%include "std_string.i"
%include "std_vector.i"
%include "std_map.i"
%include "std_multimap.i"

%{
#include "pyxis/utility/trace.h"
%}
%feature("director") TraceCallback;
%inline %{
class TraceCallback
{
public:
	virtual ~TraceCallback() {}
	virtual void message(Trace::eLevel level, const std::string& strMessage) = 0;
	static void traceCallbackAdapter(	Trace::eLevel nTraceLevel,
										const std::string& strMessage,
										void* pUserData	)
	{
		static_cast<TraceCallback*>(pUserData)->message(nTraceLevel, strMessage);
	}
};
%}

%include "std_string.i"

////////////////////////////////////////////////////////////////////////////////

%{
/* Includes the header in the wrapper code */

// So PYXObject operators are accessible (better way?)
#define SWIG_INTERNAL
#include "../../../../config/windows/force_include.h"
#include "pyxlib.h"
#include "pyxlib_instance.h"

#include "pyxis/utility/abstract_iterator.h"
#include "pyxis/utility/app_services.h"
#include "pyxis/utility/bounding_shape.h"
#include "pyxis/utility/checksum_calculator.h"
#include "pyxis/utility/command.h"
#include "pyxis/utility/command_manager.h"
#include "pyxis/utility/coord_2d.h"
#include "pyxis/utility/coord_3d.h"
#include "pyxis/utility/coord_lat_lon.h"
#include "pyxis/utility/coord_polar.h"
#include "pyxis/utility/excel.h"
#include "pyxis/utility/exception.h"
#include "pyxis/utility/exceptions.h"
#include "pyxis/utility/file_resolver.h"
#include "pyxis/utility/gdalinfo.h"
#include "pyxis/utility/great_circle_arc.h"
#include "pyxis/utility/math_utils.h"
#include "pyxis/utility/notifier.h"
#include "pyxis/utility/object.h"
#include "pyxis/utility/pointer.h"
#include "pyxis/utility/pyxcom_private.h"
#include "pyxis/utility/pyxcom.h"
#include "pyxis/utility/rect_2d.h"
#include "pyxis/utility/rgb.h"
#include "pyxis/utility/sphere_math.h"
#include "pyxis/utility/stdint.h"
#include "pyxis/utility/string_utils.h"
#include "pyxis/utility/file_utils.h"
#include "pyxis/utility/tester.h"
#include "pyxis/utility/trace.h"
#include "pyxis/utility/value.h"
#include "pyxis/utility/value_math.h"
#include "pyxis/utility/xml_transform.h"
#include "pyxis/utility/http_utils.h"
#include "pyxis/utility/bitmap_server_provider.h"

#include "pyxis/derm/child_iterator.h"
#include "pyxis/derm/neighbour_iterator.h"
#include "pyxis/derm/edge_iterator.h"
#include "pyxis/derm/coord_converter.h"
#include "pyxis/derm/cursor.h"
#include "pyxis/derm/exceptions.h"
#include "pyxis/derm/hexagon.h"
#include "pyxis/derm/horizontal_datum.h"
#include "pyxis/derm/exhaustive_iterator.h"
#include "pyxis/derm/icos_iterator.h"
#include "pyxis/derm/icosahedron.h"
#include "pyxis/derm/index.h"
#include "pyxis/derm/index_math.h"
#include "pyxis/derm/iterator.h"
#include "pyxis/derm/pentagon.h"
#include "pyxis/derm/projection_method.h"
#include "pyxis/derm/reference_sphere.h"
#include "pyxis/derm/snyder_projection.h"
#include "pyxis/derm/spiral_iterator.h"
#include "pyxis/derm/sub_index.h"
#include "pyxis/derm/sub_index_math.h"
#include "pyxis/derm/vertex_iterator.h"
#include "pyxis/derm/wgs84_coord_converter.h"
#include "pyxis/derm/wgs84.h"
#include "pyxis/derm/point_location.h"

#include "pyxis/rhombus/rhombus.h"
#include "pyxis/rhombus/rhombus_utils.h"

//Region
#include "pyxis/region/region.h"
#include "pyxis/region/vector_point_region.h"
#include "pyxis/region/circle_region.h"
#include "pyxis/region/curve_region.h"
#include "pyxis/region/multi_curve_region.h"
#include "pyxis/region/multi_polygon_region.h"
#include "pyxis/region/region_builder.h"

#include "pyxis/geometry/geometry.h"
#include "pyxis/geometry/cell.h"
#include "pyxis/geometry/test.h"
#include "pyxis/geometry/tile.h"
#include "pyxis/geometry/inner_tile_intersection_iterator.h"
#include "pyxis/geometry/inner_tile.h"
#include "pyxis/geometry/geometry_collection.h"
#include "pyxis/geometry/tile_collection.h"
#include "pyxis/geometry/icos_traverser.h"
#include "pyxis/geometry/icos_test_traverser.h"
#include "pyxis/geometry/circle_geometry.h"
#include "pyxis/geometry/multi_cell.h"
#include "pyxis/geometry/geometry_serializer.h"
#include "pyxis/geometry/vector_geometry.h"
#include "pyxis/geometry/vector_geometry2.h"

#include "pyxis/pipe/pipe_manager.h"
#include "pyxis/pipe/pipe_utils.h"
#include "pyxis/pipe/process.h"
#include "pyxis/pipe/process_init_error.h"
#include "pyxis/pipe/process_list.h"
#include "pyxis/pipe/process_spec.h"
#include "pyxis/pipe/parameter.h"
#include "pyxis/pipe/parameter_spec.h"
#include "pyxis/pipe/pipe_builder.h"
#include "pyxis/pipe/pipe_formater.h"
#include "pyxis/pipe/pyxnet_channel.h"

#include "pyxis/utility/range.h"

#include "pyxis/data/record.h"
#include "pyxis/data/record_collection.h"
#include "pyxis/data/feature.h"
#include "pyxis/data/writeable_feature.h"
#include "pyxis/data/feature_collection.h"
#include "pyxis/data/feature_collection_index.h"
#include "pyxis/data/feature_group.h"
#include "pyxis/data/coverage.h"
#include "pyxis/data/value_tile.h"
#include "pyxis/data/tile_aggregator.h"
#include "pyxis/data/pyx_feature.h"
#include "pyxis/data/histogram.h"
#include "pyxis/data/catalog.h"

#include "pyxis/query/where_condition.h"
#include "pyxis/query/where.h"

#include "pyxis/procs/cache.h"
#include "pyxis/procs/const_coverage.h"
#include "pyxis/procs/data_processor.h"
#include "pyxis/procs/path.h"
#include "pyxis/procs/bitmap.h"
#include "pyxis/procs/embedded_resource_holder.h"
#include "pyxis/procs/process_collection_proc.h"
#include "pyxis/procs/string.h"
#include "pyxis/procs/url.h"
#include "pyxis/procs/user_credentials.h"
#include "pyxis/procs/user_credentials_provider.h"
#include "pyxis/procs/viewpoint.h"
#include "pyxis/procs/tool_box_provider.h"
#include "pyxis/procs/coverage_histogram_calculator.h"
#include "pyxis/procs/geopacket_source.h"

#include "pyxis/sampling/spatial_reference_system.h"
#include "pyxis/sampling/xy_bounds_geometry.h"
#include "pyxis/sampling/xy_coverage.h"

// We get 64-bit portability warning C4267
// It's safe for us now, and might even be safe when re-SWIG'd for Win64.
// So I sent a message to swig-devel mailing list and disabled it for now.
// http://msdn2.microsoft.com/en-gb/library/6kck0s93(VS.80).aspx
#pragma warning(disable: 4267)

// For directors
#include "pyxlib_swig.h"

%}

%inline %{ 
boost::intrusive_ptr<PYXCOM_IUnknown> PYXCOMhelpCreate(REFCLSID rclsid)
{
	boost::intrusive_ptr<PYXCOM_IUnknown> spProc;
	PYXCOM_HRESULT hr = PYXCOMCreateInstance(rclsid,0,PYXCOM_IUnknown::iid,(void**) &spProc);
	if (PYXCOM_FAILED(hr))
	{
		TRACE_ERROR("Couldn't instantiate " << rclsid);
		return 0;
		//return E_FAIL;
	}
	return spProc;
}
%}

%inline %{
bool isEqualProcRef(const ProcRef& lhs, const ProcRef& rhs)
{
	return lhs == rhs;
}
%}

////////////////////////////////////////////////////////////////////////////////

// Ignore operators. We can manually re-enable the ones we want as we need.

%ignore *::operator=;
%ignore *::operator==;
%ignore *::operator!=;
%ignore *::operator<;
%ignore *::operator+=;
%ignore *::operator-=;
%ignore *::operator++;
%ignore *::operator--;
%ignore *::operator[];

%ignore operator<<;
%ignore operator>>;
%ignore operator==;
%ignore operator!=;
%ignore operator<;
%ignore operator+;
%ignore operator-;

%ignore PYXAbstractIterator::operator!;
%ignore PYXAbstractIterator::operator bool;

%ignore PYXCoord2D::operator T*;
%ignore PYXCoord3D::operator T*;

// C# code must not create PYXObjects directly, only using their create() 
// methods.
%ignore PYXTile::PYXTile(const PYXIcosIndex& index, int nCellResolution);
%ignore PYXInnerTile::PYXInnerTile(const PYXIcosIndex& index, int nCellResolution);
%ignore PYXCell::PYXCell(const PYXIcosIndex& index);

// Ignore char * overloads when there is a string overload present;
// both resolve to string on the C# side resulting in duplicate function signatures.
%ignore PYXValue::PYXValue(const char* val);
%ignore PYXValue::PYXValue(const char** pVal, int nSize);
%ignore PYXValue::PYXValue(const char** pVal, int nSize, const std::string& nullVal);
%ignore PYXValue::set(const char* val);
%ignore PYXValue::set(int n, const char* val);

//we add it manualy
%ignore PYXCOM_IUnknown::QueryInterface;

%ignore XYAsyncValueConsumer;
%ignore EnumClassObjectImpl;
%ignore FIDStr;
%ignore NoopVisitor;
%ignore makeNoopVisitor;
%ignore PreCompLatLon;
%ignore ProcessInitHelper;
%ignore PipeUtils::createProcess;
%ignore RCImpl;
%ignore AppExecutionScope;
%ignore AppServices::getExecutionScope;




////////////////////////////////////////////////////////////////////////////////

%ignore PYXIcosIndex::getSubIndex(); // overloaded const and non-const

////////////////////////////////////////////////////////////////////////////////

// Can't be defined when swig parses. (It will still be defined when swig compiles.)
#define STDMETHODCALLTYPE __stdcall
#define __stdcall

// MSVC defines its own version of inline.  They are equivalent, except that 
// SWIG doesn't know it.
#define __inline inline

%typemap(cscode) Notifier %{
    /// <summary>Delegate for notifications.</summary>
    /// <param name="spEvent"></param>
    public delegate void Notification( NotifierEvent_SPtr spEvent);

    /// <summary> 
    /// Utility class for connecting .Net delegate to C++ observer.
    /// </summary>
    private class DelegateObserver : Observer
    {
        /// <summary> The actual notification to call back.</summary>
        private Notification m_notification;
        /// <summary> The actual notification to call back.</summary>
        public Notification Notification
        {
            get { return m_notification; }
        }

        /// <summary> Constructs a DelegateObserver.</summary>
        /// <param name="notification">The callback fn.</param>
        public DelegateObserver(Notification notification)
        {
            m_notification = notification;
        }

        /// <summary> Override the "OnNotification" callback.</summary>
        /// <param name="spEvent"></param>
        protected override void updateObserverImpl(NotifierEvent_SPtr spEvent)
        {
            m_notification(spEvent);
        }
    }

    private static System.Collections.Generic.List<DelegateObserver> s_observers =
        new System.Collections.Generic.List<DelegateObserver>();

    public event Notification Event{
        add
        {
            DelegateObserver observer = new DelegateObserver(value);
            lock (s_observers)
            {
                s_observers.Add(observer);
            }
            attach(observer);
        }
        remove
        {
            lock (s_observers)
            {
                DelegateObserver observer = null;
                foreach (DelegateObserver o in s_observers)
                {
                    if (o.Notification == value)
                    {
                        observer = o;
                        break;
                    }
                }
                if (observer != null)
                {
                    detach(observer);
                    s_observers.Remove(observer);
                }
            }
        }
    }
%}    

%typemap(cscode) GUID %{
    /// <summary>Implicit conversion from GUID to Guid.</summary>
    /// <param name="g">Managed Guid</param>
    public static implicit operator Guid(GUID g)
    {
        return new Guid(g.ToString());
    }
    public override string ToString()
    {
		return pyxlib.guidToStr(this);
    }

	public override bool Equals(object guidObject)
	{
		if (null == guidObject)
		{
			return false;
		}

		GUID guid = guidObject as GUID;
		if (null != guid)
		{
			return Equals(guid);
		}

		return false;
	}

	public bool Equals(GUID guid)
	{
		return this.ToString() == guid.ToString();
	}

	public override int GetHashCode()
	{
		return this.ToString().GetHashCode();
	}

	public static bool operator ==(GUID a, GUID b)
	{
		if (System.Object.ReferenceEquals(a, b))
		{
			return true;
		}
		if (((object)a == null) || ((object)b == null))
		{
			return false;
		}
		return a.Equals(b);
	}

	public static bool operator !=(GUID a, GUID b)
	{
		return !(a==b);
	}
%}   

/* Exception handling see 17.3.2, 11.1, 11.1.3 */
%exception
%{
try
{
  $action
}
catch (PYXException& e)
{
  SWIG_CSharpSetPendingException(SWIG_CSharpApplicationException, e.getFullErrorString().c_str());
}
%}

%include "typemaps.i"

// See user manual 20.8.9
// "Here is an example which will change the getCPtr method and constructor
// from the default protected access to public access. This has a practical
// application if you are invoking SWIG more than once and generating the
// wrapped classes into different packages in each invocation. If the classes
// in one package are using the classes in another package, then these methods
// need to be public."
//%typemap(csbody) Notifier, Observer, PYXIcosIndex, PYXObject %{

// Proxy classes (base classes, ie, not derived classes)
%typemap(csbody) SWIGTYPE %{
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  public $csclassname(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = cMemoryOwn;
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr($csclassname obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }
%}

// Derived proxy classes
%typemap(csbody_derived) SWIGTYPE %{
  private HandleRef swigCPtr;

  public $csclassname(IntPtr cPtr, bool cMemoryOwn) : base($imclassname.$csclassnameUpcast(cPtr), cMemoryOwn) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  public static HandleRef getCPtr($csclassname obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }
%}

// Typewrapper classes
%typemap(csbody) SWIGTYPE *, SWIGTYPE &, SWIGTYPE [], SWIGTYPE (CLASS::*) %{
  private HandleRef swigCPtr;

  public $csclassname(IntPtr cPtr, object futureUse) {
    swigCPtr = new HandleRef(this, cPtr);
  }

  public $csclassname() {
    swigCPtr = new HandleRef(null, IntPtr.Zero);
  }

  public static HandleRef getCPtr($csclassname obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }
%}

// Instantiate templates we need
namespace std
{
  %template(Vector_Int) vector<int>;
  %template(Vector_Byte) vector<unsigned char>;
  %template(Vector_Double) vector<double>;
  %template(Vector_String) vector<std::string>;
  %template(Vector_CoordLatLon) vector<CoordLatLon>;
  %template(Vector_Index) vector<PYXIcosIndex>;  
  %template(Vector_Value) vector<PYXValue>;  
  %template(Attribute_Map) map<std::string, std::string>;
  %template(Process_Process_Map) map<boost::intrusive_ptr<IProcess>, boost::intrusive_ptr<IProcess> >;

  %template(PYXRhombus_Vector) vector<PYXRhombus>;
}

%inline %{

// TOOD[rtaylor,02/04/2013]: Remove this string encoding method when SWIG is updated
/*
Encode the string from a PYXValue in a byte array so it can be transported
across the SWIG boundary. This should no longer be required once SWIG is upgraded
to a more recent version. The Decoding back into a C# string can be done
using the StringUtilities DecodeByteVectorIntoString method.
*/
std::vector<unsigned char> getStringAsByteVector(PYXValue value, int index) 
{ 
	std::string text = value.getString(index);
	std::vector<unsigned char> returnBytes(text.begin(), text.end());
	return returnBytes;
}


std::vector<std::string> getKeysFromMap(const std::map<std::string, std::string>& map)
{
	std::vector<std::string> vec;
	for (std::map<std::string, std::string>::const_iterator it = map.begin();
		it != map.end(); ++it)
	{
		vec.push_back(it->first);
	}
	return vec;
}

std::vector<std::string> getValuesFromMap(const std::map<std::string, std::string>& map)
{
	std::vector<std::string> vec;
	for (std::map<std::string, std::string>::const_iterator it = map.begin();
		it != map.end(); ++it)
	{
		vec.push_back(it->second);
	}
	return vec;
}


std::string pathToString(boost::filesystem::path path)
{
	return FileUtils::pathToString(path);
}

boost::filesystem::path stringToPath(const std::string& pathString)
{
	return boost::filesystem::path(pathString, boost::filesystem::native);
}
%}

/* PYXLIB */

%feature("director") Observer;
%feature("director") Command;
%feature("director") CSharpFunctionProvider;
%feature("director") CSharpXMLDocProvider;
%feature("director") HttpRequestProvider;
%feature("director") ChecksumCalculator;
%feature("director") BitmapServerProvider;
%feature("director") PYXNETChannelProvider;

////////////////////////////////////////////////////////////////////////////////

namespace boost
{
  %ignore intrusive_ptr::operator=;
  %ignore intrusive_ptr::operator!;
  %ignore intrusive_ptr::operator boost::intrusive_ptr::unspecified_bool_type;
  
  %extend intrusive_ptr
  {
  	bool isNull()
	{
	    return self == nullptr || !(*self);
	}
	
	bool isNotNull()
	{
		return self != nullptr && (*self);
	}
  }
}

%import "../../../third_party/boost/boost/smart_ptr/intrusive_ptr.hpp"

%ignore PYXPointer::operator=;
%ignore PYXPointer::operator!;
%ignore PYXPointer::operator PYXPointer::unspecified_bool_type;
%import "pyxis/utility/pointer.h"
//%ignore Range<int>;
%import "pyxis/utility/range.h"

%template(Command_SPtr) PYXPointer<Command>;
%template(Command_CSPtr) PYXPointer<const Command>;
%template(NotifierEvent_SPtr) PYXPointer<NotifierEvent>;
%template(PYXIterator_SPtr) PYXPointer<PYXIterator>;
%template(PYXNeighbourIterator_SPtr) PYXPointer<PYXNeighbourIterator>;
%template(PYXChildIterator_SPtr) PYXPointer<PYXChildIterator>;
%template(PYXEdgeIterator_SPtr) PYXPointer<PYXEdgeIterator>;
%template(PYXGeometry_SPtr) PYXPointer<PYXGeometry>;
%template(PYXGeometry_CSPtr) PYXPointer<const PYXGeometry>;
%template(PYXVectorGeometry2_SPtr) PYXPointer<PYXVectorGeometry2>;
%template(PYXVectorGeometry2_CSPtr) PYXPointer<const PYXVectorGeometry2>;
%template(PYXGlobalGeometry_SPtr) PYXPointer<PYXGlobalGeometry>;
%template(PYXGlobalGeometry_CSPtr) PYXPointer<const PYXGlobalGeometry>;
%template(PYXGeometryCollection_SPtr) PYXPointer<PYXGeometryCollection>;
%template(PYXGeometryCollection_CSPtr) PYXPointer<const PYXGeometryCollection>;
%template(PYXIcosIterator_SPtr) PYXPointer<PYXIcosIterator>;
%template(PYXIcosIterator_CSPtr) PYXPointer<const PYXIcosIterator>;
%template(PYXValueTile_SPtr) PYXPointer<PYXValueTile>;
%template(PYXExhaustiveIterator_SPtr) PYXPointer<PYXExhaustiveIterator>;
%template(PYXCell_SPtr) PYXPointer<PYXCell>;
%template(PYXCell_CSPtr) PYXPointer<const PYXCell>;
%template(PYXTile_SPtr) PYXPointer<PYXTile>;
%template(PYXTile_CSPtr) PYXPointer<const PYXTile>;
%template(PYXInnerTile_SPtr) PYXPointer<PYXInnerTile>;
%template(PYXInnerTile_CSPtr) PYXPointer<const PYXInnerTile>;
%template(PYXInnerTileIntersectionIterator_SPtr) PYXPointer<PYXInnerTileIntersectionIterator>;
%template(PYXCircleGeometry_SPtr) PYXPointer<PYXCircleGeometry>;
%template(PYXCircleGeometry_CSPtr) PYXPointer<const PYXCircleGeometry>;
%template(IRegion_SPtr) PYXPointer<IRegion>;
%template(PYXVectorRegion_SPtr) PYXPointer<PYXVectorRegion>;
%template(PYXVectorPointRegion_SPtr) PYXPointer<PYXVectorPointRegion>;
%template(PYXVectorPointRegion_CSPtr) PYXPointer<const PYXVectorPointRegion>;
%template(PYXCircleRegion_SPtr) PYXPointer<PYXCircleRegion>;
%template(PYXCircleRegion_CSPtr) PYXPointer<const PYXCircleRegion>;
%template(PYXCurveRegion_SPtr) PYXPointer<PYXCurveRegion>;
%template(PYXCurveRegion_CSPtr) PYXPointer<const PYXCurveRegion>;
%template(PYXMultiCurveRegion_SPtr) PYXPointer<PYXMultiCurveRegion>;
%template(PYXMultiCurveRegion_CSPtr) PYXPointer<const PYXMultiCurveRegion>;
%template(PYXMultiPolygonRegion_SPtr) PYXPointer<PYXMultiPolygonRegion>;
%template(PYXMultiPolygonRegion_CSPtr) PYXPointer<const PYXMultiPolygonRegion>;
%template(PYXTileCollection_SPtr) PYXPointer<PYXTileCollection>;
%template(PYXTileCollection_CSPtr) PYXPointer<const PYXTileCollection>;
%template(PYXMultiCell_SPtr) PYXPointer<PYXMultiCell>;
%template(PYXMultiCell_CSPtr) PYXPointer<const PYXMultiCell>;
%template(PYXTileCollectionIterator_SPtr) PYXPointer<PYXTileCollectionIterator>;

%template(PYXWhere_SPtr) PYXPointer<PYXWhere>;
%template(PYXWhereCondition_SPtr) PYXPointer<PYXWhereCondition>;
%template(PYXCoverageWhereCondition_SPtr) PYXPointer<PYXCoverageWhereCondition>;
%template(PYXFeaturesWhereCondition_SPtr) PYXPointer<PYXFeaturesWhereCondition>;
%template(PYXGeometryWhereCondition_SPtr) PYXPointer<PYXGeometryWhereCondition>;

%template(CSharpFunctionProvider_SPtr) PYXPointer<CSharpFunctionProvider>;
%template(CSharpXMLDocProvider_SPtr) PYXPointer<CSharpXMLDocProvider>;
%ignore CSharpXMLDoc;
%template(HttpRequestProvider_SPtr) PYXPointer<HttpRequestProvider>;
%template(ChecksumCalculator_SPtr) PYXPointer<ChecksumCalculator>;
%template(ProcessSpec_SPtr) PYXPointer<ProcessSpec>;
%template(IUnknown_SPtr) boost::intrusive_ptr<PYXCOM_IUnknown>;
%template(IUnknown_CSPtr) boost::intrusive_ptr<const PYXCOM_IUnknown>;
%template(Parameter_SPtr) PYXPointer<Parameter>;
%template(Vector_GUID) std::vector<GUID>;
%template(ParameterSpec_SPtr) PYXPointer<ParameterSpec>;
%template(IProcess_SPtr) boost::intrusive_ptr<IProcess>;
%template(IProcess_CSPtr) boost::intrusive_ptr<const IProcess>;
%template(IPath_SPtr) boost::intrusive_ptr<IPath>;
%template(IBitmap_SPtr) boost::intrusive_ptr<IBitmap>;
%template(IEmbeddedResourceHolder_SPtr) boost::intrusive_ptr<IEmbeddedResourceHolder>;
%template(IUrl_SPtr) boost::intrusive_ptr<IUrl>;
%template(IUserCredentials_SPtr) boost::intrusive_ptr<IUserCredentials>;
%template(IUserCredentialsError_SPtr) boost::intrusive_ptr<IUserCredentialsError>;
%template(IUserCredentialsError_CSPtr) boost::intrusive_ptr<const IUserCredentialsError>;
%template(UserCredentialsInitError_SPtr) boost::intrusive_ptr<UserCredentialsInitError>;
%template(UserCredentialsList_SPtr) boost::intrusive_ptr<UserCredentialsList>;
%template(IUsernameAndPasswordCredentials_SPtr) boost::intrusive_ptr<IUsernameAndPasswordCredentials>;
%template(UsernameAndPasswordCredentials_SPtr) boost::intrusive_ptr<UsernameAndPasswordCredentials>;
%template(IUserCredentialsProvider_SPtr) boost::intrusive_ptr<IUserCredentialsProvider>;
%template(UserCredentialsProviderProcess_SPtr) boost::intrusive_ptr<UserCredentialsProviderProcess>;
%template(IProcessInitError_SPtr) boost::intrusive_ptr<IProcessInitError>;
%template(IProcessInitError_CSPtr) boost::intrusive_ptr<const IProcessInitError>;
%template(ProcessList_SPtr) PYXPointer<ProcessList>;
%template(IProcessCollection_SPtr) boost::intrusive_ptr<IProcessCollection>;
%template(IProcessCollection_CSPtr) boost::intrusive_ptr<const IProcessCollection>;
%template(ICache_SPtr) boost::intrusive_ptr<ICache>;
%template(ICoverage_SPtr) boost::intrusive_ptr<ICoverage>;
%template(ICoverage_CSPtr) boost::intrusive_ptr<const ICoverage>;
%template(IDataProcessor_SPtr) boost::intrusive_ptr<IDataProcessor>;
%template(IDataProcessor_CSPtr) boost::intrusive_ptr<const IDataProcessor>;
%template(IXYCoverage_SPtr) boost::intrusive_ptr<IXYCoverage>;
%template(IXYCoverage_CSPtr) boost::intrusive_ptr<const IXYCoverage>;
%template(IFeature_SPtr) boost::intrusive_ptr<IFeature>;
%template(IFeature_CSPtr) boost::intrusive_ptr<const IFeature>;
%template(IWritableFeature_SPtr) boost::intrusive_ptr<IWritableFeature>;
%template(IWritableFeature_CSPtr) boost::intrusive_ptr<const IWritableFeature>;
%template(IString_SPtr) boost::intrusive_ptr<IString>;
%template(IString_CSPtr) boost::intrusive_ptr<const IString>;
%template(ICoordConverter_SPtr) boost::intrusive_ptr<ICoordConverter>;
%template(ICoordConverter_CSPtr) boost::intrusive_ptr<const ICoordConverter>;
%template(ICoordConverterFromSrsFactory_SPtr) boost::intrusive_ptr<ICoordConverterFromSrsFactory>;
%template(ICoordConverterFromSrsFactory_CSPtr) boost::intrusive_ptr<const ICoordConverterFromSrsFactory>;
%template(PYXSpatialReferenceSystem_SPtr) boost::intrusive_ptr<PYXSpatialReferenceSystem>;

%typemap(cscode) boost::intrusive_ptr<PYXTableDefinition>, boost::intrusive_ptr<const PYXTableDefinition> %{	
	public System.Collections.Generic.IEnumerable<PYXFieldDefinition> FieldDefinitions
	{
		get
		{
			int count = getFieldCount();
			for(int i = 0; i < count; ++i)
			{
				yield return getFieldDefinition(i);
			}			
		}
	}
%}

%template(PYXTableDefinition_CSPtr) PYXPointer<const PYXTableDefinition>;
%template(PYXTableDefinition_SPtr) PYXPointer<PYXTableDefinition>;

%template(PYXDataSet_CSPtr) PYXPointer<const PYXDataSet>;
%template(PYXDataSet_SPtr) PYXPointer<PYXDataSet>;

%typemap(cscode) boost::intrusive_ptr<PYXCatalog>, boost::intrusive_ptr<const PYXCatalog> %{
	public System.Collections.Generic.IEnumerable<PYXDataSet_CSPtr> DataSets
	{
		get
		{
			int count = getDataSetCount();
			for(int i = 0; i < count; ++i)
			{
				yield return getDataSet(i);
			}			
		}
	}

	public System.Collections.Generic.IEnumerable<PYXCatalog_CSPtr> SubCatalogs
	{
		get
		{
			int count = getSubCatalogCount();
			for(int i = 0; i < count; ++i)
			{
				yield return getSubCatalog(i);
			}			
		}
	}
%}

%template(PYXCatalog_CSPtr) PYXPointer<const PYXCatalog>;
%template(PYCatalog_SPtr) PYXPointer<PYXCatalog>;

%template(IRecord_SPtr) boost::intrusive_ptr<IRecord>;
%template(IRecord_CSPtr) boost::intrusive_ptr<const IRecord>;

%typemap(cscode) IRecordCollection %{
	public System.Collections.Generic.IEnumerable<IRecord_SPtr> GetRecordsEnumerator()
	{
		RecordIterator_SPtr iterator = getIterator();
		if (iterator.isNull())
		{
			yield break;
		}
		while(!iterator.end())
		{
			yield return iterator.getRecord();
			iterator.next();
		}
	}
%}


%typemap(cscode) IFeatureCollection %{
	public System.Collections.Generic.IEnumerable<IFeature_SPtr> GetFeaturesEnumerator(PYXGeometry geometry)
	{
		FeatureIterator_SPtr iterator = getIterator(geometry);
		if (iterator.isNull())
		{
			yield break;
		}
		while(!iterator.end())
		{
			yield return iterator.getFeature();
			iterator.next();
		}
	}

	public System.Collections.Generic.IEnumerable<IFeature_SPtr> GetFeaturesEnumerator()
	{
		FeatureIterator_SPtr iterator = getIterator();
		if (iterator.isNull())
		{
			yield break;
		}
		while(!iterator.end())
		{
			yield return iterator.getFeature();
			iterator.next();
		}
	}
%}

%typemap(cscode) IFeatureGroup %{
	public System.Collections.Generic.IEnumerable<IFeature_SPtr> GetGroupEnumerator(PYXGeometry geometry)
	{
		FeatureIterator_SPtr iterator = getGroupIterator(geometry);
		if (iterator.isNull())
		{
			yield break;
		}
		while(!iterator.end())
		{
			yield return iterator.getFeature();
			iterator.next();
		}
	}

	public System.Collections.Generic.IEnumerable<IFeature_SPtr> GetGroupEnumerator()
	{
		FeatureIterator_SPtr iterator = getGroupIterator();
		if (iterator.isNull())
		{
			yield break;
		}
		while(!iterator.end())
		{
			yield return iterator.getFeature();
			iterator.next();
		}
	}
%}

%typemap(cscode) boost::intrusive_ptr<IRecordCollection>, boost::intrusive_ptr<const IRecordCollection> %{	
	public System.Collections.Generic.IEnumerable<IRecord_SPtr> GetRecordsEnumerator()
	{
		RecordIterator_SPtr iterator = getIterator();
		if (iterator.isNull())
		{
			yield break;
		}
		while(!iterator.end())
		{
			yield return iterator.getRecord();
			iterator.next();
		}
	}
%}

%typemap(cscode) boost::intrusive_ptr<IFeatureCollection>, boost::intrusive_ptr<const IFeatureCollection> %{	
	public System.Collections.Generic.IEnumerable<IFeature_SPtr> GetFeaturesEnumerator(PYXGeometry geometry)
	{
		FeatureIterator_SPtr iterator = getIterator(geometry);
		if (iterator.isNull())
		{
			yield break;
		}
		while(!iterator.end())
		{
			yield return iterator.getFeature();
			iterator.next();
		}
	}

	public System.Collections.Generic.IEnumerable<IFeature_SPtr> GetFeaturesEnumerator()
	{
		FeatureIterator_SPtr iterator = getIterator();
		if (iterator.isNull())
		{
			yield break;
		}
		while(!iterator.end())
		{
			yield return iterator.getFeature();
			iterator.next();
		}
	}
%}

%typemap(cscode) boost::intrusive_ptr<IFeatureGroup>, boost::intrusive_ptr<const IFeatureGroup> %{

	public System.Collections.Generic.IEnumerable<IFeature_SPtr> GetFeaturesEnumerator(PYXGeometry geometry)
	{
		FeatureIterator_SPtr iterator = getIterator(geometry);
		if (iterator.isNull())
		{
			yield break;
		}
		while(!iterator.end())
		{
			yield return iterator.getFeature();
			iterator.next();
		}
	}

	public System.Collections.Generic.IEnumerable<IFeature_SPtr> GetFeaturesEnumerator()
	{
		FeatureIterator_SPtr iterator = getIterator();
		if (iterator.isNull())
		{
			yield break;
		}
		while(!iterator.end())
		{
			yield return iterator.getFeature();
			iterator.next();
		}
	}

	public System.Collections.Generic.IEnumerable<IFeature_SPtr> GetGroupEnumerator(PYXGeometry geometry)
	{
		FeatureIterator_SPtr iterator = getGroupIterator(geometry);
		if (iterator.isNull())
		{
			yield break;
		}
		while(!iterator.end())
		{
			yield return iterator.getFeature();
			iterator.next();
		}
	}

	public System.Collections.Generic.IEnumerable<IFeature_SPtr> GetGroupEnumerator()
	{
		FeatureIterator_SPtr iterator = getGroupIterator();
		if (iterator.isNull())
		{
			yield break;
		}
		while(!iterator.end())
		{
			yield return iterator.getFeature();
			iterator.next();
		}
	}
%}

%template(IRecordCollection_SPtr) boost::intrusive_ptr<IRecordCollection>;
%template(IRecordCollection_CSPtr) boost::intrusive_ptr<const IRecordCollection>;
%template(RecordIterator_SPtr) PYXPointer<RecordIterator>;
%template(IFeatureCollection_SPtr) boost::intrusive_ptr<IFeatureCollection>;
%template(IFeatureCollection_CSPtr) boost::intrusive_ptr<const IFeatureCollection>;
%template(IFeatureCollectionIndex_SPtr) boost::intrusive_ptr<IFeatureCollectionIndex>;
%template(IFeatureCollectionIndex_CSPtr) boost::intrusive_ptr<const IFeatureCollectionIndex>;
%template(FeatureIterator_SPtr) PYXPointer<FeatureIterator>;
%template(IFeatureGroup_SPtr) boost::intrusive_ptr<IFeatureGroup>;
%template(IFeatureGroup_CSPtr) boost::intrusive_ptr<const IFeatureGroup>;
%template(ICoverageHistogramCalculator_SPtr) boost::intrusive_ptr<ICoverageHistogramCalculator>;
%template(ICoverageHistogramCalculator_CSPtr) boost::intrusive_ptr<const ICoverageHistogramCalculator>;
%template(IPipeBuilder_SPtr) boost::intrusive_ptr<IPipeBuilder>;
%template(IPipeFormater_SPtr) boost::intrusive_ptr<IPipeFormater>;
%template(PYXFeature_SPtr) boost::intrusive_ptr<PYXFeature>;
%template(PathInitError_SPtr) boost::intrusive_ptr<PathInitError>;
%template(PipeBuilderBase_SPtr) boost::intrusive_ptr<PipeBuilderBase>;
%template(IViewPoint_SPtr) boost::intrusive_ptr<IViewPoint>;
%template(ProcessResolver_SPtr) PYXPointer<ProcessResolver>;
%template(BitmapServerProvider_SPtr) PYXPointer<BitmapServerProvider>;
%template(PYXXYBoundsGeometry_SPtr) PYXPointer<PYXXYBoundsGeometry>;
%template(IToolBoxProvider_SPtr) boost::intrusive_ptr<IToolBoxProvider>;
%template(IToolBoxProvider_CSPtr) boost::intrusive_ptr<const IToolBoxProvider>;

%template(PYXNETChannelProvider_SPtr) PYXPointer<PYXNETChannelProvider>;
%template(PYXNETChannel_SPtr) PYXPointer<PYXNETChannel>;
%template(PYXNETChannelKeyProvider_SPtr) PYXPointer<PYXNETChannelKeyProvider>;

%template(PYXHistogram_SPtr) boost::intrusive_ptr<PYXHistogram>;
%template(PYXHistogram_CSPtr) boost::intrusive_ptr<const PYXHistogram>;
%template(PYXCellHistogram_SPtr) boost::intrusive_ptr<PYXCellHistogram>;
%template(PYXCellHistogram_CSPtr) boost::intrusive_ptr<const PYXCellHistogram>;
%template(PYXHistogramBinVector) std::vector<PYXHistogramBin>;
%template(PYXCellHistogramBinVector) std::vector<PYXCellHistogramBin>;
%template(RangeInt) Range<int>;
%template(RangeDouble) Range<double>;
%template(RangeString) Range<std::string>;
%template(RangePYXValue) Range<PYXValue>;


%template(CacheNeedsTileEvent_SPtr) PYXPointer<CacheNeedsTileEvent>;
%template(CacheWithProcessEvent_SPtr) PYXPointer<CacheWithProcessEvent>;
%template(CommandManagerEvent_SPtr) PYXPointer<CommandManagerEvent>;
%template(ProcessEvent_SPtr) PYXPointer<ProcessEvent>;
%template(ProcessListEvent_SPtr) PYXPointer<ProcessListEvent>;
%template(ProcessProcessingEvent_SPtr) PYXPointer<ProcessProcessingEvent>;
%template(CommandExecutedEvent_SPtr) PYXPointer<CommandExecutedEvent>;
%template(FileEvent_SPtr) PYXPointer<FileEvent>;
%template(PipelineFilesEvent_SPtr) PYXPointer<PipelineFilesEvent>;
%template(ViewPointChangedEvent_SPtr) PYXPointer<ViewPointChangedEvent>;


// Below here are vectors of intrusive_ptr

SWIG_STD_VECTOR_SPECIALIZE_MINIMUM(IProcess_SPtr, boost::intrusive_ptr<IProcess>)
namespace std
{
  %template(Vector_IProcess) vector< boost::intrusive_ptr<IProcess> >;
  %template(Vector_ProcRef) vector< ProcRef >;  
}

SWIG_STD_VECTOR_SPECIALIZE_MINIMUM(IFeature_SPtr, boost::intrusive_ptr<IFeature>)
namespace std
{
  %template(Vector_IFeature) vector< boost::intrusive_ptr<IFeature> >;
}

SWIG_STD_VECTOR_SPECIALIZE_MINIMUM(IUnknown_SPtr, boost::intrusive_ptr<PYXCOM_IUnknown>)
namespace std
{
  %template(Vector_IUnknown) vector< boost::intrusive_ptr<PYXCOM_IUnknown> >;
}


SWIG_STD_VECTOR_SPECIALIZE_MINIMUM(Parameter_SPtr, PYXPointer<Parameter>)
namespace std
{
  %template(Vector_Parameter) vector< PYXPointer<Parameter> >;
}


SWIG_STD_VECTOR_SPECIALIZE_MINIMUM(Command_SPtr, PYXPointer<Command>)
namespace std
{
	%template(Vector_Command) vector< PYXPointer<Command> >;
}

////////////////////////////////////////////////////////////////////////////////

%define FEATURE_INTRUSIVE_PROXY(BaseType)
// Proxy classes (base classes, ie, not derived classes)
%typemap(csbody) BaseType %{
  private HandleRef swigCPtr;
  protected bool swigCMemOwn;

  public $csclassname(IntPtr cPtr, bool cMemoryOwn) {
    swigCMemOwn = false;
    swigCPtr = new HandleRef(this, cPtr);
    $module.intrusive_ptr_add_ref(this);
  }

  public static HandleRef getCPtr($csclassname obj) {
    return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
  }
%}

%typemap(csdestruct, methodname="Dispose", methodmodifiers="public") BaseType {
    lock(this) {
      if(swigCPtr.Handle != IntPtr.Zero) {
        $module.intrusive_ptr_release(this);
      }
      swigCPtr = new HandleRef(null, IntPtr.Zero);
      GC.SuppressFinalize(this);
    }
  }
%enddef

%define FEATURE_INTRUSIVE_PROXY_AS(BaseType, RefType)
%inline %{
//! Helper function for boost::intrusive_ptr which adds a reference to an object.
inline void intrusive_ptr_add_ref(BaseType const * const p)
{
	intrusive_ptr_add_ref(static_cast<RefType const * const>(p));
}

//! Helper function for boost::intrusive_ptr which releases an object.
inline void intrusive_ptr_release(BaseType const * p)
{
	intrusive_ptr_release(static_cast<RefType const * const>(p));
}
%}
FEATURE_INTRUSIVE_PROXY(BaseType)
%enddef

FEATURE_INTRUSIVE_PROXY(PYXCOM_IUnknown)
%ignore FeatureIterator;
FEATURE_INTRUSIVE_PROXY_AS(NotifierEvent, PYXObject)
FEATURE_INTRUSIVE_PROXY_AS(Parameter, PYXObject)
FEATURE_INTRUSIVE_PROXY_AS(ParameterSpec, PYXObject)
FEATURE_INTRUSIVE_PROXY_AS(ProcessList, PYXObject)
FEATURE_INTRUSIVE_PROXY_AS(ProcessSpec, PYXObject)
FEATURE_INTRUSIVE_PROXY_AS(PYXGeometry, PYXObject)
%ignore PYXGeometryIterator;
FEATURE_INTRUSIVE_PROXY_AS(PYXIterator, PYXObject)
%ignore PYXXYIterator;
%ignore TestTile;

////////////////////////////////////////////////////////////////////////////////

// Rename methods that only differ by const in the main interfaces
%rename(getOutput_const) IProcess::getOutput() const;
%rename(getGeometry_const) IFeature::getGeometry() const;
%rename(getDataSetDefinition_const) PYXDataSet::getDataSetDefinition() const;
%rename(getContentDefinition_const) PYXDataSet::getContentDefinition() const;

////////////////////////////////////////////////////////////////////////////////

// Ignore these classes simply to suppress SWIG warning 401
%ignore PYXObject;
class PYXObject {};
%ignore PYXGeometryIterator;
class PYXGeometryIterator {};

////////////////////////////////////////////////////////////////////////////////

%include "pyxlib.h"
%include "pyxlib_instance.h"
#define BOOST_FILESYSTEM_NAMESPACE filesystem
#define BOOST_FILESYSTEM_DECL
#define BOOST_WINDOWS_API
#define BOOST_FILESYSTEM_NARROW_ONLY
namespace boost{
%ignore filesystem::path::begin;
%ignore filesystem::path::end;
%ignore filesystem::path::string;
%rename filesystem::path::string ToString;
%ignore filesystem::path::path( const char * src );
%ignore filesystem::path::path( const char * src, name_check checker );
}

//%import "../../../third_party/boost/boost/filesystem/path.hpp"
//%include "../../../third_party/boost/boost/filesystem/path.hpp"
//#include "../../../third_party/boost/boost/filesystem/path.hpp"

//namespace boost {
//namespace filesystem{
// %ignore BoostPath::begin;
// %template(BoostPath) path;
//}
//}

%extend PYXWhere
{
public:
	PYXPointer<PYXWhere> intersect(const PYXPointer<PYXCoverageWhereCondition> & condition)
	{
		return self->intersect(condition);
	}

	PYXPointer<PYXWhere> intersect(const PYXPointer<PYXFeaturesWhereCondition> & condition)
	{
		return self->intersect(condition);
	}

	PYXPointer<PYXWhere> intersect(const PYXPointer<PYXGeometryWhereCondition> & condition)
	{
		return self->intersect(condition);
	}

	PYXPointer<PYXWhere> subtract(const PYXPointer<PYXCoverageWhereCondition> & condition)
	{
		return self->subtract(condition);
	}

	PYXPointer<PYXWhere> subtract(const PYXPointer<PYXFeaturesWhereCondition> & condition)
	{
		return self->subtract(condition);
	}

	PYXPointer<PYXWhere> subtract(const PYXPointer<PYXGeometryWhereCondition> & condition)
	{
		return self->subtract(condition);
	}

	PYXPointer<PYXWhere> disjunct(const PYXPointer<PYXCoverageWhereCondition> & condition)
	{
		return self->disjunct(condition);
	}

	PYXPointer<PYXWhere> disjunct(const PYXPointer<PYXFeaturesWhereCondition> & condition)
	{
		return self->disjunct(condition);
	}

	PYXPointer<PYXWhere> disjunct(const PYXPointer<PYXGeometryWhereCondition> & condition)
	{
		return self->disjunct(condition);
	}
}


%extend AppServices
{
public:
	static std::string makeTempFile(
		const std::string& strExtension = std::string(""))
	{
		return FileUtils::pathToString(AppServices::makeTempFile(strExtension)); 
	}

	//! Create a temporary directory.
	static std::string makeTempDir()
	{
		return FileUtils::pathToString(AppServices::makeTempDir()); 
	}

	//! Get the trace file path.
	static std::string getTraceFilePath()
	{
		return FileUtils::pathToString(AppServices::getTraceFilePath()); 
	}

	//! Create or fetch a cache directory by name.
	static std::string getCacheDir(const std::string& strDirName)
	{
		return FileUtils::pathToString(AppServices::getCacheDir(strDirName)); 
	}


	//! Return the path to the executable directory.
	static std::string getApplicationPath()
	{
		return FileUtils::pathToString(AppServices::getApplicationPath()); 
	}


	//! Return the path to the working directory.
	static std::string getWorkingPath()
	{
		return FileUtils::pathToString(AppServices::getWorkingPath()); 
	}


	//! Return the path to the library directory.
	static std::string getLibraryPath()
	{
		return FileUtils::pathToString(AppServices::getLibraryPath()); 
	}


	//! Return the path to the base cache directory.
	static std::string getBaseCachePath()
	{
		return FileUtils::pathToString(AppServices::getBaseCachePath()); 
	}

}

//we simple override those functions. ignoring them just remove them fro the SWIG interface
//and rename didn't work as well
%ignore AppServices::makeTempFile;
%ignore AppServices::makeTempDir;
%ignore AppServices::getTraceFilePath;
%ignore AppServices::getCacheDir;
%ignore AppServices::getWorkingPath;
%ignore AppServices::getApplicationPath;
%ignore AppServices::getBaseCachePath;
%ignore AppServices::getLibraryPath;

// Utilities
%include "pyxis/utility/abstract_iterator.h"
%include "pyxis/utility/app_services.h"

%template(getAppProperty) getAppProperty<std::string>;

%include "pyxis/utility/bounding_shape.h"
%include "pyxis/utility/checksum_calculator.h"
%include "pyxis/utility/command.h"
%include "pyxis/utility/command_manager.h"


////////////////////////////////////////////////////////////////////////////////

%typemap(cscode) PYXCoord2D<double> %{
	public override string ToString()
	{
		return String.Format("({0},{1})",this.x(),this.y());
	}
%}

%typemap(cscode) CoordLatLon %{
	public override string ToString()
	{
		return String.Format("(lat:{0},lon:{1})",this.latInDegrees(),this.lonInDegrees());
	}
%}

////////////////////////////////////////////////////////////////////////////////

%include "pyxis/utility/coord_2d.h"
%include "pyxis/utility/coord_3d.h"
%include "pyxis/utility/coord_lat_lon.h"
%include "pyxis/utility/coord_polar.h"
%include "pyxis/utility/exception.h"
%include "pyxis/utility/exceptions.h"
%include "pyxis/utility/file_resolver.h"
%include "pyxis/utility/gdalinfo.h"
%include "pyxis/utility/great_circle_arc.h"
%include "pyxis/utility/math_utils.h"
%include "pyxis/utility/notifier.h"
%include "pyxis/utility/pyxcom_private.h"
%include "pyxis/utility/pyxcom.h"
%include "pyxis/utility/rect_2d.h"
%include "pyxis/utility/rgb.h"
%include "pyxis/utility/sphere_math.h"
%include "pyxis/utility/stdint.h"
%ignore readHex;
%ignore writeHex;
%include "pyxis/utility/string_utils.h"
%include "pyxis/utility/tester.h"
%include "pyxis/utility/trace.h"
%include "pyxis/utility/value.h"
%include "pyxis/utility/value_math.h"
%include "pyxis/utility/xml_transform.h"
%include "pyxis/utility/http_utils.h"
%include "pyxis/utility/bitmap_server_provider.h"
////////////////////////////////////////////////////////////////////////////////

// Derm
%include "pyxis/derm/exceptions.h"
%include "pyxis/derm/hexagon.h"
%include "pyxis/derm/pentagon.h"
%include "pyxis/derm/spiral_iterator.h"
//%include "pyxis/derm/icosahedron.h"                 // Face nested class
//%include "pyxis/derm/horizontal_datum.h"
%include "pyxis/derm/reference_sphere.h"

%typemap(cscode) PYXIcosIndex %{
	public override bool Equals(object indexObject)
	{
		if (null == indexObject)
		{
			return false;
		}

		PYXIcosIndex index = indexObject as PYXIcosIndex;
		if (null != index)
		{
			return Equals(index);
		}

		return false;
	}

	public bool Equals(PYXIcosIndex index)
	{
		return this.toString() == index.toString();
	}

	public override int GetHashCode()
	{
		return this.toString().GetHashCode();
	}

	public override string ToString()
	{
		return this.toString();
	}

	public static bool operator ==(PYXIcosIndex a, PYXIcosIndex b)
	{
		if (System.Object.ReferenceEquals(a, b))
		{
			return true;
		}
		if (((object)a == null) || ((object)b == null))
		{
			return false;
		}
		return a.Equals(b);
	}

	public static bool operator !=(PYXIcosIndex a, PYXIcosIndex b)
	{
		return !(a==b);
	}
%}

// Indexes
%apply int *INOUT {PYXMath::eHexDirection *};
%include "pyxis/derm/sub_index.h"
%apply int *INOUT {int *pnMoveA};
%apply int *INOUT {int *pnMoveB};
%include "pyxis/derm/sub_index_math.h"
%clear int *pnMoveA;
%clear int *pnMoveB;
%include "pyxis/derm/index.h"
%include "pyxis/derm/index_math.h"

// Snyder projection
%include "pyxis/derm/coord_converter.h"
%include "pyxis/derm/projection_method.h"
%include "pyxis/derm/snyder_projection.h"
%include "pyxis/derm/point_location.h"
%include "pyxis/derm/wgs84_coord_converter.h"
%include "pyxis/derm/wgs84.h"

// Iterators and cursors
%copyctor PYXCursor;
%include "pyxis/derm/cursor.h"
%include "pyxis/derm/iterator.h"
%include "pyxis/derm/child_iterator.h"
%include "pyxis/derm/neighbour_iterator.h"
%include "pyxis/derm/vertex_iterator.h"
%include "pyxis/derm/edge_iterator.h"
%include "pyxis/derm/icos_iterator.h"
%include "pyxis/derm/exhaustive_iterator.h"


%apply int * OUTPUT { int * coordU, int * coordV};
%apply double * OUTPUT { double * coordU, double * coordV};
%include "pyxis/rhombus/rhombus.h"
%include "pyxis/rhombus/rhombus_utils.h"

//Region
%include "pyxis/region/region.h"
%include "pyxis/region/vector_point_region.h"
%include "pyxis/region/circle_region.h"
%include "pyxis/region/curve_region.h"
%include "pyxis/region/multi_curve_region.h"
%include "pyxis/region/multi_polygon_region.h"
%include "pyxis/region/region_builder.h"

// Geometry
%include "pyxis/geometry/geometry.h"
%include "pyxis/geometry/cell.h"
%include "pyxis/geometry/test.h"
%include "pyxis/geometry/tile.h"
%include "pyxis/geometry/inner_tile.h"
%include "pyxis/geometry/inner_tile_intersection_iterator.h"
%include "pyxis/geometry/geometry_collection.h"
%include "pyxis/geometry/tile_collection.h"
%include "pyxis/geometry/icos_traverser.h"
%include "pyxis/geometry/icos_test_traverser.h"
%include "pyxis/geometry/multi_cell.h"
%include "pyxis/geometry/circle_geometry.h"
%include "pyxis/geometry/vector_geometry.h"
%include "pyxis/geometry/vector_geometry2.h"


//
// NOTE: Geometries seralize them self to binary data strings.
//       SWIG think all string are Utf8. and it doesn't like binary data strings.
//       Therefrore:
//         1. we ignore all serialize/deserialize functions.
//         2. we extend the c++ class to serializeToBase64/deserializeFromBase64 functions
//         3. we extend the C# class with new serialize/deserialize functions that
//            are using the serializeToBase64/deserializeFromBase64.
//
// Why we do all that: we covert to Base64 before we pass a string throught SWIG,
// a Base64 string is safe to pass throught SWIG and the inner data can be anything.
// 
%ignore PYXGeometrySerializer::serialize;
%ignore PYXGeometrySerializer::deserialize;

%extend PYXGeometrySerializer
{

public:
	static PYXPointer<PYXGeometry> deserializeFromBase64(const std::string& strIn)
	{
		return PYXGeometrySerializer::deserialize(XMLUtils::fromBase64(strIn));
	}

	static std::string serializeToBase64(const PYXGeometry& geometry)
	{
		return XMLUtils::toBase64(PYXGeometrySerializer::serialize(geometry));
	}
}

%typemap(csbody) PYXGeometrySerializer %{
	private HandleRef swigCPtr;
	protected bool swigCMemOwn;

	public $csclassname(IntPtr cPtr, bool cMemoryOwn) {
		swigCMemOwn = cMemoryOwn;
		swigCPtr = new HandleRef(this, cPtr);
	}

	public static HandleRef getCPtr($csclassname obj) {
		return (obj == null) ? new HandleRef(null, IntPtr.Zero) : obj.swigCPtr;
	}

	public static PYXGeometry_SPtr deserialize(string strIn)
	{
		char[] chars = strIn.ToCharArray();

		System.Collections.Generic.List<byte> bytes = new System.Collections.Generic.List<byte>();

		foreach (char c in chars)
		{
			if (c < 0x100)
			{
				bytes.Add((byte)c);
			}
			else
			{
				bytes.Add((byte)(c / 256));
				bytes.Add((byte)(c % 256));
			}
		}

		byte[] toEncodeAsBytes = bytes.ToArray();

		return PYXGeometrySerializer.deserializeFromBase64(System.Convert.ToBase64String(toEncodeAsBytes));
	}

	public static string serialize(PYXGeometry geometry)
	{
		string base64steam = PYXGeometrySerializer.serializeToBase64(geometry);

		byte[] encodedDataAsBytes = System.Convert.FromBase64String(base64steam);

		char[] chars = new char[encodedDataAsBytes.Length];

		for (int i = 0; i < chars.Length; i++)
		{
			chars[i] = (char)encodedDataAsBytes[i];
		}

		return new String(chars);
	}
%}

%include "pyxis/geometry/geometry_serializer.h"

// Pipe
%apply const std::string& {std::string* pStr};

//swig fail to covert to utf8 the char* - removing them from the api
%ignore PipeManager::readProcessFromString(const char* pStr, int nStrSize);

//swig fail to covert to utf8 the char* - removing them from the api
%ignore PipeManager::readPipelineFromString(const char* pStr, int nStrSize);

%include "pyxis/pipe/pipe_manager.h"

%typemap(cscode) ProcRef %{
	public override bool Equals(object procRefObject)
	{
		if (null == procRefObject)
		{
			return false;
		}

		ProcRef procRef = procRefObject as ProcRef;
		if (null != procRef)
		{
			return Equals(procRef);
		}

		return false;
	}

	public bool Equals(ProcRef procRef)
	{
		return pyxlib.isEqualProcRef(this,procRef);
	}

	public override int GetHashCode()
	{
		return pyxlib.procRefToStr(this).GetHashCode();
	}

	public override string ToString()
	{
		return pyxlib.procRefToStr(this);
	}

	public static bool operator ==(ProcRef a, ProcRef b)
	{
		if (System.Object.ReferenceEquals(a, b))
		{
			return true;
		}
		if (((object)a == null) || ((object)b == null))
		{
			return false;
		}
		return a.Equals(b);
	}

	public static bool operator !=(ProcRef a, ProcRef b)
	{
		return !(a==b);
	}
%}

%clear pStr;
%include "pyxis/pipe/pipe_utils.h"
%include "pyxis/pipe/process_spec.h"
%include "pyxis/pipe/process.h"
%include "pyxis/pipe/process_init_error.h"
%include "pyxis/pipe/process_list.h"
%include "pyxis/pipe/parameter.h"
%include "pyxis/pipe/parameter_spec.h"
%include "pyxis/pipe/pipe_builder.h"
%include "pyxis/pipe/pipe_formater.h"
%include "pyxis/pipe/pyxnet_channel.h"

// data
%include "pyxis/data/record.h"
%include "pyxis/data/record_collection.h"
%include "pyxis/data/feature.h"
%include "pyxis/data/writeable_feature.h"
%include "pyxis/data/feature_collection.h"
%include "pyxis/data/feature_collection_index.h"
%include "pyxis/data/feature_group.h"
%include "pyxis/data/coverage.h"
%include "pyxis/data/value_tile.h"
%include "pyxis/data/tile_aggregator.h"
%include "pyxis/data/pyx_feature.h"
%include "pyxis/data/histogram.h"
%include "pyxis/data/catalog.h"

// Query

%include "pyxis/query/where_condition.h"
%include "pyxis/query/where.h"

// Procs
%include "pyxis/procs/cache.h"
%include "pyxis/procs/const_coverage.h"
%include "pyxis/procs/data_processor.h"
%include "pyxis/procs/path.h"
%include "pyxis/procs/bitmap.h"
%include "pyxis/procs/embedded_resource_holder.h"
%include "pyxis/procs/process_collection_proc.h"
%include "pyxis/procs/string.h"
%include "pyxis/procs/url.h"
%include "pyxis/procs/user_credentials.h"
%include "pyxis/procs/user_credentials_provider.h"
%include "pyxis/procs/viewpoint.h"
%include "pyxis/procs/tool_box_provider.h"
%include "pyxis/procs/coverage_histogram_calculator.h"
%include "pyxis/procs/geopacket_source.h"

// Sampling
%include "pyxis/sampling/spatial_reference_system.h"
%include "pyxis/sampling/xy_bounds_geometry.h"
%include "pyxis/sampling/xy_coverage.h"

%extend IPipeBuilder
{
	
public:
	%{
		bool bBuiltDirectory;
	%}
	
	bool get(bool &boolValue)
	{
	    boolValue = bBuiltDirectory;
		return true;
	}
	
	bool set(const bool &boolValue) 
	{
	  bBuiltDirectory = boolValue;
	  return true;
	}
	
	bool builtDirectory()
	{
		return bBuiltDirectory;
	}
	
};


%extend PipeManager
{
public:
    static std::string writeProcessToNewString(boost::intrusive_ptr<IProcess> spProc)
    {
		std::string str;
		PipeManager::writeProcessToString(&str, spProc);
		return str;
    }
    static std::string writePipelineToNewString(boost::intrusive_ptr<IProcess> spPipe)
    {
		std::string str;
		PipeManager::writePipelineToString(&str, spPipe);
		return str;
    }
}

%extend IProcess
{
public:
	bool ProvidesOutputType(GUID guid)
	{
		void * test = 0;
		self->getOutput()->QueryInterface(guid,&test);
		if (test != 0)
		{
			self->Release();
		}
		return test != 0;
	}
}

////////////////////////////////////////////////////////////////////////////////

%template(PYXCoord2DInt) PYXCoord2D<int>;
%template(PYXCoord3DInt) PYXCoord3D<int>;
%template(PYXCoord2DDouble) PYXCoord2D<double>;
%template(PYXCoord3DDouble) PYXCoord3D<double>;


%template(procRefToStr) StringUtils::toString<ProcRef>;
%template(strToPYXValue) StringUtils::fromString<PYXValue>;
%template(PYXRect2DDouble) PYXRect2D<double>;

%template(GdalInfoDataSourceNodeList) std::vector<GdalInfoDataSourceNode*>;

////////////////////////////////////////////////////////////////////////////////
%extend GUID{
public:
    void ParseString( const std::string &s)
    {
        GUID result = strToGuid( s);
        memcpy( self, &result, sizeof( GUID));
    }
}

%extend Trace
{
	static bool enabled(Trace::eLevel nLevel)
	{
		return TRACE_ENABLED(nLevel);
	}
	static void debug(const std::string& strMessage)
	{
		if (TRACE_ENABLED(Trace::knDebug)) Trace::message(Trace::knDebug, "C#", -1, strMessage);
	}
	static void error(const std::string& strMessage)
	{
		if (TRACE_ENABLED(Trace::knError)) Trace::message(Trace::knError, "C#", -1, strMessage);
	}
	static void info(const std::string& strMessage)
	{
		if (TRACE_ENABLED(Trace::knInfo)) Trace::message(Trace::knInfo, "C#", -1, strMessage);
	}
	static void memory(const std::string& strMessage)
	{
		if (TRACE_ENABLED(Trace::knMemory)) Trace::message(Trace::knMemory, "C#", -1, strMessage);
	}
	static void notify(const std::string& strMessage)
	{
		if (TRACE_ENABLED(Trace::knNotify)) Trace::message(Trace::knNotify, "C#", -1, strMessage);
	}
	static void thread(const std::string& strMessage)
	{
		if (TRACE_ENABLED(Trace::knThread)) Trace::message(Trace::knThread, "C#", -1, strMessage);
	}
	static void time(const std::string& strMessage)
	{
		if (TRACE_ENABLED(Trace::knTime)) Trace::message(Trace::knTime, "C#", -1, Trace::getInstance()->getTimestampString() + strMessage);
	}
	static void test(const std::string& strMessage)
	{
		if (TRACE_ENABLED(Trace::knTest)) Trace::message(Trace::knTest, "C#", -1, strMessage);
	}
	static void ui(const std::string& strMessage)
	{
		if (TRACE_ENABLED(Trace::knUI)) Trace::message(Trace::knUI, "C#", -1, strMessage);
	}
	static void uiDebug(const std::string& strMessage)
	{
		if (TRACE_ENABLED(Trace::knUIDebug)) Trace::message(Trace::knUIDebug, "C#", -1, strMessage);
	}
	
	static void setTraceCallback(TraceCallback* pTraceCallback)
	{
		Trace::setCallback(pTraceCallback ? TraceCallback::traceCallbackAdapter : 0, pTraceCallback);
	}
};

////////////////////////////////////////////////////////////////////////////////

// See user manual 20.9.7 Adding Java downcasts to polymorphic return types
%define FEATURE_DYNAMIC_CAST(DestType, SourceType)
%exception DestType::dynamic_cast(SourceType* pSource)
{
	// TODO maybe we shouldn't completely override global exception handler,
	// but somehow extend it?
	$action
	if (!result)
	{
		// TODO this should be InvalidCastException
		SWIG_CSharpSetPendingException(
			SWIG_CSharpInvalidCastException,
			"C++ dynamic_cast<"
			#DestType
			">("
			#SourceType
			") failed"); // seems that this needs to be on separate line
	}
}

%extend DestType
{
	static DestType* dynamic_cast(SourceType* pSource)
	{
		return dynamic_cast<DestType*>(pSource);
	}
};
%enddef

////////////////////////////////////////////////////////////////////////////////

// See http://www.boost.org/libs/smart_ptr/intrusive_ptr.html#dynamic_pointer_cast
%define FEATURE_DYNAMIC_POINTER_CAST(P, T, U)
  %inline %{
    P<T> DynamicPointerCast_ ## T(const P<U>& sp)
    {
      return boost::dynamic_pointer_cast<T, U>(sp);
    }
    P<U> DynamicPointerCast_ ## U(const P<T>& sp)
    {
      return boost::dynamic_pointer_cast<U, T>(sp);
    }   

    P<const T> DynamicPointerCast_const_ ## T(const P<const U>& sp)
    {
      return boost::dynamic_pointer_cast<const T, const U>(sp);
    }
    P<const U> DynamicPointerCast_const_ ## U(const P<const T>& sp)
    {
      return boost::dynamic_pointer_cast<const U, const T>(sp);
    }   

  %}
%enddef

FEATURE_DYNAMIC_POINTER_CAST(PYXPointer, PYXIcosIterator, PYXIterator)
FEATURE_DYNAMIC_POINTER_CAST(PYXPointer, PYXCell, PYXGeometry)
FEATURE_DYNAMIC_POINTER_CAST(PYXPointer, PYXTile, PYXGeometry)
FEATURE_DYNAMIC_POINTER_CAST(PYXPointer, PYXMultiCell, PYXGeometry)
FEATURE_DYNAMIC_POINTER_CAST(PYXPointer, PYXTileCollection, PYXGeometry)
FEATURE_DYNAMIC_POINTER_CAST(boost::intrusive_ptr, IFeature, IRecord)
FEATURE_DYNAMIC_POINTER_CAST(boost::intrusive_ptr, IWritableFeature, IFeature)
FEATURE_DYNAMIC_POINTER_CAST(PYXPointer, PYXGeometry, PYXGeometryCollection)
FEATURE_DYNAMIC_POINTER_CAST(PYXPointer, PYXGeometry, PYXGlobalGeometry)
FEATURE_DYNAMIC_POINTER_CAST(PYXPointer, PYXGeometry, PYXXYBoundsGeometry)
FEATURE_DYNAMIC_POINTER_CAST(boost::intrusive_ptr, ICoverage, PYXCOM_IUnknown)
FEATURE_DYNAMIC_POINTER_CAST(boost::intrusive_ptr, IDataProcessor, PYXCOM_IUnknown)
FEATURE_DYNAMIC_POINTER_CAST(boost::intrusive_ptr, IProcess, PYXCOM_IUnknown)
FEATURE_DYNAMIC_POINTER_CAST(PYXPointer, PYXGeometryCollection, PYXTileCollection)
FEATURE_DYNAMIC_POINTER_CAST(boost::intrusive_ptr, IViewPoint, PYXCOM_IUnknown)
FEATURE_DYNAMIC_POINTER_CAST(boost::intrusive_ptr, IPath, IUrl)
FEATURE_DYNAMIC_POINTER_CAST(boost::intrusive_ptr, IBitmap, PYXCOM_IUnknown)
FEATURE_DYNAMIC_POINTER_CAST(boost::intrusive_ptr, IBitmap, IProcess)
FEATURE_DYNAMIC_POINTER_CAST(boost::intrusive_ptr, IEmbeddedResourceHolder, PYXCOM_IUnknown)
FEATURE_DYNAMIC_POINTER_CAST(boost::intrusive_ptr, IEmbeddedResourceHolder, IProcess)
FEATURE_DYNAMIC_POINTER_CAST(PYXPointer, PYXCellHistogram, PYXHistogram)
FEATURE_DYNAMIC_POINTER_CAST(PYXPointer, PYXVectorGeometry2, PYXGeometry)
FEATURE_DYNAMIC_POINTER_CAST(PYXPointer, PYXVectorRegion, IRegion)
FEATURE_DYNAMIC_POINTER_CAST(PYXPointer, PYXVectorPointRegion, IRegion)
FEATURE_DYNAMIC_POINTER_CAST(PYXPointer, PYXCircleRegion, IRegion)
FEATURE_DYNAMIC_POINTER_CAST(PYXPointer, PYXCurveRegion, IRegion)
FEATURE_DYNAMIC_POINTER_CAST(PYXPointer, PYXMultiCurveRegion, IRegion)
FEATURE_DYNAMIC_POINTER_CAST(PYXPointer, PYXMultiPolygonRegion, IRegion)

FEATURE_DYNAMIC_POINTER_CAST(PYXPointer, PYXCoverageWhereCondition, PYXWhereCondition)
FEATURE_DYNAMIC_POINTER_CAST(PYXPointer, PYXFeaturesWhereCondition, PYXWhereCondition)
FEATURE_DYNAMIC_POINTER_CAST(PYXPointer, PYXGeometryWhereCondition, PYXWhereCondition)

FEATURE_DYNAMIC_CAST(CacheNeedsTileEvent, NotifierEvent)
FEATURE_DYNAMIC_CAST(CacheWithProcessEvent, NotifierEvent)
FEATURE_DYNAMIC_CAST(CommandManagerEvent, NotifierEvent)
FEATURE_DYNAMIC_CAST(ProcessEvent, NotifierEvent)
FEATURE_DYNAMIC_CAST(ProcessListEvent, NotifierEvent)
FEATURE_DYNAMIC_CAST(ProcessProcessingEvent, NotifierEvent)
FEATURE_DYNAMIC_CAST(CommandExecutedEvent, NotifierEvent)
FEATURE_DYNAMIC_CAST(FileEvent, NotifierEvent)
FEATURE_DYNAMIC_CAST(PipelineFilesEvent, NotifierEvent)
FEATURE_DYNAMIC_CAST(ViewPointChangedEvent, NotifierEvent)

////////////////////////////////////////////////////////////////////////////////

%define FEATURE_QUERY_INTERFACE(T, U)
  %inline %{
    boost::intrusive_ptr<T> QueryInterface_ ## T(const boost::intrusive_ptr<U>& sp)
    {
      boost::intrusive_ptr<T> sp2;
	  if (sp)
	  {
        sp->QueryInterface(T::iid, (void**) &sp2);
      }
      return sp2;
    }
    boost::intrusive_ptr<U> QueryInterface_ ## U(const boost::intrusive_ptr<T>& sp)
    {
      boost::intrusive_ptr<U> sp2;
	  if (sp)
	  {
        sp->QueryInterface(U::iid, (void**) &sp2);
	  }
      return sp2;
    }
    boost::intrusive_ptr<const T> QueryInterface_ ## T ## _const(const boost::intrusive_ptr<const U>& sp)
    {
      boost::intrusive_ptr<const T> sp2;
	  if (sp)
	  {
        boost::const_pointer_cast<U, const U>(sp)->QueryInterface(T::iid, (void**) &sp2);
	  }
      return sp2;
    }
    boost::intrusive_ptr<const U> QueryInterface_ ## U ## _const(const boost::intrusive_ptr<const T>& sp)
    {
      boost::intrusive_ptr<const U> sp2;
	  if (sp)
	  {
		boost::const_pointer_cast<T, const T>(sp)->QueryInterface(U::iid, (void**) &sp2);
      }
      return sp2;
    }
  %}
%enddef

FEATURE_QUERY_INTERFACE(PYXCOM_IUnknown, IProcess)
FEATURE_QUERY_INTERFACE(PYXCOM_IUnknown, IFeature)
FEATURE_QUERY_INTERFACE(PYXCOM_IUnknown, IWritableFeature)
FEATURE_QUERY_INTERFACE(PYXCOM_IUnknown, IRecordCollection)
FEATURE_QUERY_INTERFACE(PYXCOM_IUnknown, IFeatureCollection)
FEATURE_QUERY_INTERFACE(PYXCOM_IUnknown, IFeatureCollectionIndex)
FEATURE_QUERY_INTERFACE(PYXCOM_IUnknown, IFeatureGroup)
FEATURE_QUERY_INTERFACE(PYXCOM_IUnknown, ICoverageHistogramCalculator)
FEATURE_QUERY_INTERFACE(PYXCOM_IUnknown, ICoverage)
FEATURE_QUERY_INTERFACE(PYXCOM_IUnknown, ICoordConverterFromSrsFactory)
FEATURE_QUERY_INTERFACE(PYXCOM_IUnknown, ICache)
FEATURE_QUERY_INTERFACE(PYXCOM_IUnknown, IString)
FEATURE_QUERY_INTERFACE(PYXCOM_IUnknown, IProcessCollection)
FEATURE_QUERY_INTERFACE(PYXCOM_IUnknown, IXYCoverage)
FEATURE_QUERY_INTERFACE(PYXCOM_IUnknown, IPipeBuilder)
FEATURE_QUERY_INTERFACE(PYXCOM_IUnknown, IPipeFormater)
FEATURE_QUERY_INTERFACE(PYXCOM_IUnknown, IDataProcessor)
FEATURE_QUERY_INTERFACE(PYXCOM_IUnknown, IPath)
FEATURE_QUERY_INTERFACE(PYXCOM_IUnknown, IUrl)
FEATURE_QUERY_INTERFACE(PYXCOM_IUnknown, IUserCredentials)
FEATURE_QUERY_INTERFACE(PYXCOM_IUnknown, IUsernameAndPasswordCredentials)
FEATURE_QUERY_INTERFACE(PYXCOM_IUnknown, IUserCredentialsProvider)
FEATURE_QUERY_INTERFACE(PYXCOM_IUnknown, IBitmap)
FEATURE_QUERY_INTERFACE(PYXCOM_IUnknown, IEmbeddedResourceHolder)
FEATURE_QUERY_INTERFACE(PYXCOM_IUnknown, IViewPoint)
FEATURE_QUERY_INTERFACE(PYXCOM_IUnknown, IToolBoxProvider)

// TODO: review the rest of these Query Interfaces as they may indicate 
// incorrect usage.
FEATURE_QUERY_INTERFACE(IProcess, IDataProcessor)
FEATURE_QUERY_INTERFACE(IProcess, IPath)
FEATURE_QUERY_INTERFACE(IProcess, IUrl)
FEATURE_QUERY_INTERFACE(IProcess, IUserCredentialsProvider)
FEATURE_QUERY_INTERFACE(IProcess, IBitmap)
FEATURE_QUERY_INTERFACE(IProcess, IEmbeddedResourceHolder)
FEATURE_QUERY_INTERFACE(IProcess, IToolBoxProvider)

FEATURE_QUERY_INTERFACE(IRecord, IFeatureCollection)
FEATURE_QUERY_INTERFACE(IRecord, IRecordCollection)
FEATURE_QUERY_INTERFACE(IFeature, ICoverage)
FEATURE_QUERY_INTERFACE(IFeature, IRecord)
FEATURE_QUERY_INTERFACE(IFeature, IFeatureCollection)
FEATURE_QUERY_INTERFACE(IFeature, IFeatureGroup)
FEATURE_QUERY_INTERFACE(IWritableFeature, IFeature)
FEATURE_QUERY_INTERFACE(IFeatureCollection, IFeatureGroup)

FEATURE_QUERY_INTERFACE(IUserCredentials, IUsernameAndPasswordCredentials)
FEATURE_QUERY_INTERFACE(IProcessInitError, IUserCredentialsError)

////////////////////////////////////////////////////////////////////////////////

%feature("director") Excel::IWorkbookView;
%feature("director") Excel::IWorkbookTable;
%feature("director") Excel::IWorkbook;
%feature("director") Excel::IExcel;

%template(IWorkbookViewPointer) PYXPointer<Excel::IWorkbookView const>;
%template(IWorkbookTablePointer) PYXPointer<Excel::IWorkbookTable const>;
%template(IWorkbookPointer) PYXPointer<Excel::IWorkbook const>;

#undef PYXLIB_DECL
#define PYXLIB_DECL

%include "pyxis/utility/excel.h"
