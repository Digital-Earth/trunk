using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;

namespace Pyxis.Contract.Workspaces.Domains
{
    public class DateRangeDomain : IDomain
    {
        public string Type 
        {
            get
            {
                return "date";    
            }    
        }

        public bool Contains(string value)
        {
            var date = DateTime.Parse(value, null, DateTimeStyles.AssumeUniversal).ToUniversalTime();

            if (date < From || date > To)
            {
                return false;
            }

            return EnumarateTime(FindNearestDate(date)).Contains(date);
        }

        public enum TimeSteps
        {
            P1Y,
            P1M,
            P7D,
            P2D,
            P1D,
            PT12H,
            PT8H,
            PT6H,
            PT4H,
            PT3H,
            PT2H,
            PT1H,
            PT30M,
            PT20M,
            PT15M,
            PT10M,
            PT5M,
        };

        public DateTime From { get; set; }
        public DateTime To { get; set; }
        public TimeSteps Step { get; set; }

        public DateRangeDomain(DateTime from, DateTime to, string step)
            : this(from, to, (TimeSteps) (Enum.Parse(typeof(TimeSteps), step)))
        {
            
        }

        public DateRangeDomain(DateTime from, DateTime to, TimeSteps step)
        {
            From = from;
            To = to;
            Step = step;
        }

        public IEnumerable<string> Values
        {
            get
            {
                foreach (var date in EnumarateDates())
                {
                    foreach (var time in EnumarateTime(date))
                    {
                        if (time <= To)
                        {
                            yield return time.ToString("yyyy-MM-ddTHH:mm:ssZ");
                        }
                    }
                }
            }
        }

        private DateTime FindNearestDate(DateTime day)
        {
            var date = From;

            switch (Step)
            {
                case TimeSteps.P1Y:
                    date = date.AddYears(day.Year - date.Year);
                    break;

                case TimeSteps.P1M:
                    date = date.AddYears(day.Year - date.Year);
                    date = date.AddMonths(day.Month - date.Month);
                    break;

                case TimeSteps.P7D:
                    date = date.AddDays((day - date).Days / 7);
                    break;

                case TimeSteps.P2D:
                    date = date.AddDays((day - date).Days / 2);
                    break;

                case TimeSteps.P1D:
                default:
                    date = date.AddDays((day - date).Days);
                    break;
            }
            return date;
        }

        private IEnumerable<DateTime> EnumarateDates()
        {
            var date = From;

            while (date <= To)
            {
                yield return date;
                switch (Step)
                {
                    case TimeSteps.P1Y:
                        date = date.AddYears(1);
                        break;

                    case TimeSteps.P1M:
                        date = date.AddMonths(1);
                        break;

                    case TimeSteps.P7D:
                        date = date.AddDays(7);
                        break;

                    case TimeSteps.P2D:
                        date = date.AddDays(2);
                        break;

                    case TimeSteps.P1D:
                    default:
                        date = date.AddDays(1);
                        break;
                }
            }
        }

        private IEnumerable<DateTime> EnumarateTime(DateTime day)
        {
            var time = day;

            while (time.Date == day.Date)
            {
                yield return time;
                switch (Step)
                {
                    case TimeSteps.PT12H:
                        time = time.AddHours(12);
                        break;

                    case TimeSteps.PT8H:
                        time = time.AddHours(8);
                        break;

                    case TimeSteps.PT6H:
                        time = time.AddHours(6);
                        break;

                    case TimeSteps.PT4H:
                        time = time.AddHours(4);
                        break;

                    case TimeSteps.PT3H:
                        time = time.AddHours(3);
                        break;

                    case TimeSteps.PT2H:
                        time = time.AddHours(2);
                        break;

                    case TimeSteps.PT1H:
                        time = time.AddHours(1);
                        break;

                    case TimeSteps.PT30M:
                        time = time.AddMinutes(30);
                        break;

                    case TimeSteps.PT20M:
                        time = time.AddMinutes(20);
                        break;

                    case TimeSteps.PT15M:
                        time = time.AddMinutes(15);
                        break;

                    case TimeSteps.PT10M:
                        time = time.AddMinutes(10);
                        break;

                    case TimeSteps.PT5M:
                        time = time.AddMinutes(5);
                        break;

                    default:
                        yield break;
                }
            }
        }

        public string FormatValue(string value, string format = null)
        {
            var date = DateTime.Parse(value, null, DateTimeStyles.AssumeUniversal).ToUniversalTime();

            switch (format)
            {
                case "YYYY":
                    return date.ToString("yyyy");
                case "YYYYMMDD":
                    return date.ToString("yyyyMMdd");
                case "MM":
                    return date.ToString("MM");
                case "DD":
                    return date.ToString("dd");
                default:
                    return date.ToString("yyyy-MM-ddTHH:mm:ssZ");
            }
        }
    }
}