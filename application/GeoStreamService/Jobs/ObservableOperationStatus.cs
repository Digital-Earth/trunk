/******************************************************************************
OperationStatus.cs

begin		: Oct. 8, 2013
copyright	: (C) 2013 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

using System;
using Pyxis.Contract.Operations;
using Pyxis.Publishing.Protocol.ContractObligations;
using Pyxis.Utilities;

namespace GeoStreamService.Jobs
{
    public class ObservableOperationStatus : OperationStatus
    {
        public ObservableOperationStatus()
        {
            Operation = new Operation();
            Guid = System.Guid.NewGuid().ToString();
        }

        private DateTime m_endTime;

        public new DateTime EndTime
        {
            get
            {
                return m_endTime;
            }
            set
            {
                if (m_endTime != value)
                {
                    m_endTime = value;
                    RaiseStatusChanged();
                }
            }
        }

        private float? m_progress;

        public new float? Progress
        {
            get
            {
                return m_progress;
            }
            set
            {
                if (m_progress != value)
                {
                    m_progress = value;
                    RaiseStatusChanged();
                }
            }
        }

        private void RaiseStatusChanged()
        {
            System.Threading.Tasks.Task.Factory.StartNew(() => StatusChanged.SafeInvoke(this));
        }

        private DateTime m_startTime;

        public new DateTime StartTime
        {
            get
            {
                return m_startTime;
            }
            set
            {
                if (m_startTime != value)
                {
                    m_startTime = value;
                    RaiseStatusChanged();
                }
            }
        }

        private OperationStatusCode m_statusCode;

        public override OperationStatusCode StatusCode
        {
            get
            {
                return m_statusCode;
            }
            set
            {
                if (m_statusCode != value)
                {
                    m_statusCode = value;
                    RaiseStatusChanged();
                }
            }
        }

        public event EventHandler StatusChanged;
    }
}