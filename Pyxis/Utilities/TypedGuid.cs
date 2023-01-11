// TypedGuid.cs
//
// Author: Darius Zakrzewski
// Copyright: (c) 2003 Darius Zakrzewski

namespace System
{
    // Base class for strongly typed Guid classes.
    //
    // The TypedGuid class represents a class-ified Guid structure. It wraps a Guid instance
    // and replicates most public methods available on the .NET Guid structure. It is an
    // abstract base class, as creating instances of it would defeat its purpose. It provides
    // a protected constructor that takes a Guid argument.
    //
    // Strongly typed Guid classes are created by deriving subclasses from the TypedGuid class.
    // The subclass constructor takes a single Guid argument and calls the base class constructor.
    // In most cases the subclasses can be almost empty. The association is implied through the
    // type of the subclass.

    [Serializable]
    public abstract class TypedGuid : IComparable, IFormattable
    {
        private /*readonly*/ Guid m_guid;

        protected TypedGuid(Guid guid)
        {
            m_guid = guid;
        }

        protected TypedGuid()
        {
            m_guid = new Guid();
        }

        public Guid Guid
        {
            get
            {
                return m_guid;
            }
            set
            {
                m_guid = value;
            }
        }

        public static implicit operator Guid(TypedGuid typedGuid)
        {
            return typedGuid.Guid;
        }

        public override int GetHashCode()
        {
            return m_guid.GetHashCode();
        }

        // Value equality. Comparisons with Guid instances are allowed, but
        // fail the .NET Framework Class Library Object.Equals() statement
        // that "x.Equals(y) returns the same value as y.Equals(x)".
        public override bool Equals(object obj)
        {
            if (Object.ReferenceEquals(obj, null))
                return false;

            if (Object.ReferenceEquals(obj, this))
                return true;

            if (obj is TypedGuid) 
                return m_guid.Equals(((TypedGuid)obj).m_guid);
           
            if (obj is Guid)
                return m_guid.Equals((Guid)obj);

            return false;
        }

        // Value equality. Strict implementation of the .NET Framework Class Library
        // IComparable.CompareTo() statement that "parameter, obj, must be the same
        // type as the class [..] that implements this interface".
        public virtual int CompareTo(object obj) 
        {
            if (Object.ReferenceEquals(obj, null))
                return 1;

            if (Object.ReferenceEquals(obj, this))
                return 0;

            if(GetType() != obj.GetType())
                throw new ArgumentException(String.Format( // a resource string would be ideal
                    "Parameter {0} is not of type {1}.", obj.GetType(), GetType()));

            return m_guid.CompareTo(((TypedGuid)obj).m_guid);
        }

        public static bool operator==(TypedGuid lhs, TypedGuid rhs)
        {
            return Object.Equals(lhs, rhs);
        }

        public static bool operator!=(TypedGuid lhs, TypedGuid rhs)
        {			
            return !(lhs == rhs);
        }

        public override string ToString()
        {
            return m_guid.ToString();
        }

        public virtual string ToString(string format)
        {
            return m_guid.ToString(format);
        }

        public virtual string ToString(string format, IFormatProvider formatProvider)
        {
            return m_guid.ToString(format, formatProvider);
        }

        public virtual byte[] ToByteArray()
        {
            return m_guid.ToByteArray();
        }
    }    
}
