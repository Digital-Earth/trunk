/* 
- @name resourceResolver
- @desc a service that resolve resources from the gallery (and generate specification for GeoSources)
- @type {service}
*/
app.service("resourceResolver", function (dispatcher, $pyx) {
    var service = {
        /*
        - @name resolve
        - @desc resolve a resource from worldview.gallery
        - @param {resourceReference} resourceRefernece - resource reference: Id + Version
        - @type {function}
        */
        'resolve': function (resourceReference) {
            $pyx.gallery.resources().getByIdAndVersion(resourceReference.Id, resourceReference.Version).success(function (resource) {
                dispatcher.actions.resourceResolvedCompleted.safeInvoke({ 'resource': resource });
            }).error(function (error) {
                dispatcher.actions.resourceResolvedFailed.safeInvoke({ 'resource': resourceReference, 'error': error });
            });
        },
        /*
        - @name resolveSpecification
        - @desc resolve the specification for a given GeoSource
        - @param {geoSource} geoSource - geoSource without Specification
        - @type {function}
        */
        'resolveSpecification': function (geoSource) {
            $pyx.engine.getSpecification(geoSource).success(function (specification) {
                dispatcher.actions.resourceSpecificationResolvedCompleted.safeInvoke({ 'resource': geoSource, 'specification': specification });
            }).error(function (error) {
                dispatcher.actions.resourceSpecificationResolvedFailed.safeInvoke({ 'resource': geoSource, 'error': error });
            });
        }
    };
    
    return service;
});