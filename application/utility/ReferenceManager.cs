using System;
using System.Collections.Generic;
using System.Text;

namespace ApplicationUtility
{    
    /// <summary>
    /// Interface to be implemented to work with the ReferenceManager.        
    /// </summary>
    public interface IDirectorReferenceCounter
    {
        /// <summary>
        /// Called the base.addRef of the director to apply addRef in the c++ world.
        /// </summary>
        /// <returns>The new reference counting after applying the addRef method</returns>
        int doAddRef();

        /// <summary>
        /// Called the base.release of the director to apply release in the c++ world.
        /// </summary>
        /// <returns>The new reference counting after applying the release method</returns>
        int doRelease();

        /// <summary>
        /// Change the value swigCMemOwn of the object.
        /// 
        /// The swigCMemOwn telling the swig C# objet whether to kill the C++ object or not.
        /// The ReferenceManager would change the SwigCMemOwn flag depend on the amount of C++ unmanaged reference the object has.        
        /// </summary>
        /// <param name="value">the new value of siwgCMemOwn</param>
        void setSwigCMemOwn(bool value);
    }

    /// <summary>
    /// Helper class to Manintain Refernece for all director classes
    /// 
    /// This class keep a static pointer to all objects that have ref count > 1.
    /// 
    /// The way a Director should use this help class is as follows:
    /// 
    /// class MyManagedDirector : DirectorBaseClass, IDirectorReferenceCounter
    /// {
    /// 
    ///     #region IDirectorReferenceCounter Members
    ///  
    ///     public void setSwigCMemOwn(bool value)
    ///     {
    ///         swigCMemOwn = value;
    ///     }
    ///     public int doAddRef()
    ///     {
    ///         return base.addRef();
    ///     }
    /// 
    ///     public int doRelease()
    ///     {
    ///         return base.release();
    ///     }
    ///     
    ///     #endregion
    /// 
    ///     ...
    ///    
    ///     public virtual int addRef()
    ///     {
    ///        return ReferenceManager.addRef(this);
    ///     }
    ///    
    ///     public virtual int release()
    ///     {
    ///        return ReferenceManager.release(this);
    ///     }   
    /// }
    /// </summary>
    public class ReferenceManager
    {
        static List<IDirectorReferenceCounter> staticReferences = new List<IDirectorReferenceCounter>();

        public static int addRef(IDirectorReferenceCounter obj)
        {
            lock (obj)
            {
                int refCount = obj.doAddRef();

                if (refCount == 2)
                {
                    lock (staticReferences)
                    {
                        staticReferences.Add(obj);
                    }
                    obj.setSwigCMemOwn(false);
                }
                return refCount;
            }
        }

        public static int release(IDirectorReferenceCounter obj)
        {
            lock (obj)
            {
                int refCount = obj.doRelease();

                if (refCount == 1)
                {
                    lock (staticReferences)
                    {
                        staticReferences.Remove(obj);
                    }

                    obj.setSwigCMemOwn(true);
                }
                return refCount;
            }
        }
    }
}
