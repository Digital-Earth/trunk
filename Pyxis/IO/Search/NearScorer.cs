namespace Pyxis.IO.Search
{
    public class NearScorer : IGazetteerScorer
    {
        private PYXBoundingCircle m_boundingCircle;

        public NearScorer(PYXBoundingCircle circle)
        {
            m_boundingCircle = circle;
        }

        public float Score(IGazetteerEntry entry)
        {
            var entryWithBoundingCircle = entry as GazetteerEntry;

            if (entryWithBoundingCircle == null || entryWithBoundingCircle.BoundingCircle == null || m_boundingCircle == null)
            {
                return 0;
            }

            var distanceBetweenCentersInRadians =
                PointLocation.fromXYZ(m_boundingCircle.getCenter())
                    .distance(PointLocation.fromXYZ(entryWithBoundingCircle.BoundingCircle.getCenter())) / SphereMath.knEarthRadius;

            var twoRadiuses = m_boundingCircle.getRadius() + entryWithBoundingCircle.BoundingCircle.getRadius();

            if (distanceBetweenCentersInRadians < m_boundingCircle.getRadius())
            {
                return 1;
            }
            else if (distanceBetweenCentersInRadians > twoRadiuses)
            {
                return 0;
            }
            else
            {
                return (float)(distanceBetweenCentersInRadians / twoRadiuses);
            }
        }
    }
}