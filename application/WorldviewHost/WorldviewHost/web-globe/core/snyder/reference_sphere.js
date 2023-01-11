/******************************************************************************
reference_sphere.js

begin		: 2015-12-23
copyright	: (C) 2015 by the PYXIS innovation inc.
web			: www.pyxisinnovation.com
******************************************************************************/

'use strict';

function defineReferenceSphere() {

    //! Singleton instance
    var ReferenceSphere = {};

    //! The radius of the sphere in metres from GRS 1980 Authalic Sphere (EPSG_code=7048)
    ReferenceSphere.RADIUS = 6371007.0;

    /*!
    Calculate the distance in metres between two points.

    \param	pt1	The first point.
    \param	pt2	The second point.

    \return	The distance in metres.
    */
    /*
    double ReferenceSphere::calcDistance(	const CoordLatLon& pt1,
									        const CoordLatLon& pt2	) const
    {
        return GreatCircleArc::calcDistance(pt1, pt2, kfRadius);
    }
    */

    return ReferenceSphere;
};

module.exports = defineReferenceSphere();
