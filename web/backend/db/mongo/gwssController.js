/*************************************
* Backend JS Gwss controller
* The purpose of this script is to
* update the LS requests to the
* Gwsses.  Other auxiliary collections
* are generated but not to be used 
* externally.
*************************************/

function purgeInactiveServers (config) {
    db.Gwsses.remove({ "LastHeard": { $lt: new Date(new Date().getTime() - config.cutoffMilliseconds) } });
}

function getGwssesView () {
    var gwssesCursor = db.Gwsses.find({ Group: { $ne: "Debug" } });
    var gwsses = {};
    gwssesCursor.forEach(function (gwss) {
        if (!gwss.Request || gwss.Group === "Publishing") {
            gwss.Request = {
                PipelinesRequests: [],
                Counts: {
                    Publish: 0,
                    Remove: 0
                }
            };
        } 
        gwsses[gwss._id] = gwss;
    });
    return gwsses;
}

Object.filter = function (obj, predicate) {
    var result = {}
    var key;
    for (key in obj) {
        if (obj.hasOwnProperty(key) && predicate(obj[key])) {
            result[key] = obj[key];
        }
    }
    return result;
};

function getActiveServedProcRefs () {
    return db.Resources.distinct("ProcRef", { "Type": "GeoSource", "State": { $in: ["Active", "Broken"] }, "Metadata.SystemTags": { $nin: ["OGC", "WebService", "GeoServices"] } });
}

/**
 * Remove requests that each gwss had made sufficient progress on
 * For Publish requests the gwss reported status of the pipeline is used
 * For Remove requests the absence of the pipeline in the status is used
 */
function removeProgressedRequests (gwsses) {
    for (var gwssId in gwsses) {
        if (gwsses.hasOwnProperty(gwssId)) {
            var gwss = gwsses[gwssId];
            var publishRequests = gwss.Request.PipelinesRequests.filter(function (pipelineRequest) {
                return pipelineRequest.OperationType === "Publish";
            });
            var progressedRequests = [];
            var pipelineStatuses = gwss.Status.PipelinesStatuses;
            for (var i = 0; i < pipelineStatuses.length; i++) {
                var progressed = publishRequests.some(function (request) {
                    return pipelineStatuses[i].ProcRef === request.Parameters.ProcRef
                        && pipelineStatuses[i].StatusCode !== "Removing";
                });
                if (progressed) {
                    progressedRequests.push(pipelineStatuses[i].ProcRef);
                }
                if (progressedRequests.length === publishRequests.length) {
                    break;
                }
            }
            gwss.Request.PipelinesRequests = gwss.Request.PipelinesRequests.filter(function (pipelineRequest) {
                if (pipelineRequest.OperationType === "Publish") {
                    return !progressedRequests.some(function (progressedRequest) {
                        return progressedRequest === pipelineRequest.Parameters.ProcRef;
                    });
                } else if (pipelineRequest.OperationType === "Remove") {
                    // Removes progressed if they are not included in the pipelines statuses
                    return pipelineStatuses.some(function(pipeline) {
                        return pipeline === pipelineRequest.Parameters.ProcRef;
                    });
                }
            });
        }
    }
}

/**
 * Insert outstanding requests from the last LS-GWSS request dispatch into 
 * the Pipelines collection for a consistent view so that requests that have 
 * not progressed are sent to the same gwss
 */
function insertOutstandingRequests (gwsses) {
    var bulk = db.Pipelines.initializeUnorderedBulkOp();
    for (var gwssId in gwsses) {
        if (gwsses.hasOwnProperty(gwssId)) {
            gwsses[gwssId].Request.PipelinesRequests.forEach(function (request) {
                if (request.OperationType === "Publish") {
                    bulk.find({ _id: request.Parameters.ProcRef }).updateOne({ $addToSet: { Servers: gwsses[gwssId]._id }, $inc: { Count: 1 } });
                } else if(request.OperationType === "Remove") {
                    bulk.find({ _id: request.Parameters.ProcRef }).updateOne({ $pull: { Servers: gwsses[gwssId]._id }, $inc: { Count: -1 } });
                }
            });
        }
    }
    bulk.execute();
}

/**
 * Generate a collection storing for each pipeline the servers that are publishing or 
 * have been requested to publish it
 */
function generatePipelinesCollection (gwsses, activeServedProcRefs) {
    db.Gwsses.aggregate([
    	{ $match: { $and: [{ "Group": { $ne: "Publishing" } }, { "Group": { $ne: "Debug" } }] } },
        { $unwind: "$Status.PipelinesStatuses" },
        { $match: { "Status.PipelinesStatuses.StatusCode": { $ne: "Removing" } } },
        { $group: { _id: "$Status.PipelinesStatuses.ProcRef", Count: { $sum: 1 }, Servers: { $push: "$_id" } } },
        { $out: "Pipelines" }
    ]);
    // insert (without overwriting) any ProcRefs not being served
    var bulk = db.Pipelines.initializeUnorderedBulkOp();
    activeServedProcRefs.forEach(function (procRef) {
        bulk.find({ _id: procRef }).upsert().updateOne({ $setOnInsert: { Count: 0, Servers: [] } });
    })
    bulk.execute();

    removeProgressedRequests(gwsses);
    insertOutstandingRequests(gwsses);
}

function initializeGwssRequestCounts (gwsses) {
    for (var gwssId in gwsses) {
        if (gwsses.hasOwnProperty(gwssId)) {
            var gwss = gwsses[gwssId]
            gwss.Request.Counts = {
                Publish: 0,
                Remove: 0
            };
            gwss.Request.PipelinesRequests.forEach(function(request) {
                if (request.OperationType === "Remove") {
                    ++gwss.Request.Counts.Remove;
                } else if (request.OperationType === "Publish") {
                    ++gwss.Request.Counts.Publish;
                }
            });
        }
    }
}

function estimateSpaceChange(config, pipelineSizeMB) {
    return config.spaceSafetyMargin * pipelineSizeMB;
}

/**
 * Estimate the available disk space of each gwss using it's last reported amount of space
 * and estimating the space required to fulfill outstanding requests
 */
function getGwssesAvailableSpace(gwsses, config) {
    var gwssesAvailableSpace = [];
    var requestedPipelines = [];
    var included = {};
    for (var gwssId in gwsses) {
        if (gwsses.hasOwnProperty(gwssId)) {
            var gwss = gwsses[gwssId];
            gwssesAvailableSpace.push({ "_id": gwss._id, "Space": gwsses[gwssId].Status.ServerStatus.AvailableDiskSpaceMB });
            gwss.Request.PipelinesRequests.forEach(function(request) {
                if (request.OperationType !== "Publish") {
                    return;
                }
                var procRef = request.Parameters.ProcRef;
                if (!included.hasOwnProperty(procRef)) {
                    included[procRef] = [gwss._id];
                    requestedPipelines.push(procRef);
                } else {
                    included[procRef].push(gwss._id);
                }
            });
        }
    }
    var pipelinesDetails = db.Resources.find({ "Type": "GeoSource", "ProcRef": { $in: requestedPipelines } }, { "_id": 0, "ProcRef": 1, "DataSize": 1 }).toArray();
    var gwssAvailableSpaceMap = {};
    var bytesPerMB = 1024 * 1024;
    gwssesAvailableSpace.forEach(function(gwss) {
        gwssAvailableSpaceMap[gwss._id] = gwss;
    });
    pipelinesDetails.forEach(function(pipeline) {
        included[pipeline.ProcRef].forEach(function(gwssId) {
            gwssAvailableSpaceMap[gwssId].Space -= estimateSpaceChange(config, pipeline.DataSize / bytesPerMB);
        });
    });

    return gwssesAvailableSpace;
}

/**
 * Upsert a request of operationType for procRef into gwssRequest
 * updating request Counts appropriately
 */
function upsertRequest(gwssRequest, procRef, operationType) {
    var requests = gwssRequest.PipelinesRequests;
    var requestIndex;
    for (requestIndex = 0; requestIndex < requests.length; requestIndex++) {
        if (requests[requestIndex].Parameters.ProcRef === procRef) {
            break;
        }
    }
    if (requestIndex === requests.length) {
        // _t is the magic deserializing hint for the C# BSON deserializer
        requests.push({ "_t": "Operation", OperationType: operationType, Parameters: { ProcRef: procRef } });
    } else {
        gwssRequest.Counts[requests[requestIndex].OperationType]--;
        requests[requestIndex].OperationType = "Remove";
    }
    gwssRequest.Counts[operationType]++;
}

function removeNonActiveServedPipelines (gwsses, activeServedProcRefs) {
    var removedPipelines = db.Pipelines.find({ "_id": { $nin: activeServedProcRefs } });
    var bulkRemovePipelines = [];
    removedPipelines.forEach(function (pipelineServers) {
        var removedPipeline = pipelineServers._id;
        pipelineServers.Servers.forEach(function (serverId) {
            upsertRequest(gwsses[serverId].Request, removedPipeline, "Remove");
        });
    });
    db.Pipelines.remove({ "_id": { $nin: activeServedProcRefs } });
}

/**
 * Create a map for a pipeline property addressable by ProcRef
 */
function generatePipelinesMap (property, pipelinesDetails) {
    var pipelinesMap = {};
    pipelinesDetails.forEach(function (pipelineDetails) {
        pipelinesMap[pipelineDetails.ProcRef] = pipelineDetails[property];
    });
    return pipelinesMap;
}

/**
 * Create a map for the number of hosts required to server a pipeline addressable by ProcRef
 */
function generatePipelinesHostsMap (pipelinesDetails, minimumHosts, slaHosts) {
    var pipelinesHostsMap = {};
    pipelinesDetails.forEach(function (pipelineDetails) {
        var numHosts = minimumHosts;
        var systemTags = pipelineDetails.Metadata.SystemTags;
        for (var i = 0; i < systemTags.length; i++) {
            if (systemTags[i].startsWith("SLA-")) {
                numHosts = slaHosts[systemTags[i]];
            }
        }
        pipelinesHostsMap[pipelineDetails.ProcRef] = numHosts;
    });
    return pipelinesHostsMap;
}

function objectIdIndexOf (ids, id) {
    for (var idsIndex = 0; idsIndex < ids.length; idsIndex++) {
        if (ids[idsIndex].base64() == id.base64()) {
            return idsIndex;
        }
    }
    return -1;
}

/**
 * Add requests to gwsses that are able to publish pipelines that are not sufficiently served 
 */
function publishInsufficentlyServedPipelines (gwsses, gwssesAvailableSpace, config) {
    var numServers = gwssesAvailableSpace.length;
    var bytesPerMB = 1024 * 1024;
    var pipelinesToServe = db.Pipelines.find({ "Count": { $lt: config.maximumHosts } }).sort({ Count: 1 }).toArray();
    var pipelinesProcRefs = db.Pipelines.distinct("_id", { "Count": { $lt: config.maximumHosts } });
    var pipelinesDetails = db.Resources.find({ "Type": "GeoSource", "ProcRef": { $in: pipelinesProcRefs } }, { "_id": 0, "ProcRef": 1, "DataSize": 1, "Metadata.SystemTags": 1 }).toArray();
    var pipelinesSizeMap = generatePipelinesMap("DataSize", pipelinesDetails);
    var pipelinesHostsMap = generatePipelinesHostsMap(pipelinesDetails, config.minimumHosts, config.slaHosts);

    var madeAllocation;
    // attempt to allocate pipelines until an iteration when no allocations could be made
    // This can be due to any of hitting the publish request limit, server space limits, or no pipelines to serve
    do {
        madeAllocation = false;
        // attempt to allocate one server per pipeline and repeat
        pipelinesToServe.forEach(function(pipelineServers) {
            var pipelineToServe = pipelineServers._id;
            var numServersNeeded = pipelinesHostsMap[pipelineToServe] - pipelineServers.Servers.length;
            if (numServersNeeded <= 0) {
                return;
            }
            // add to the server with the fewest requests and most space first
            gwssesAvailableSpace.sort(function(a, b) {
                var aRequests = gwsses[a._id].Request.Counts.Publish;
                var bRequests = gwsses[b._id].Request.Counts.Publish;
                return aRequests - bRequests
                    || (aRequests == bRequests && b.Space - a.Space);
            });
            var pipelineSizeMB = pipelinesSizeMap[pipelineToServe] / bytesPerMB;
            for (var serverIndex = 0; numServersNeeded > 0 && serverIndex < numServers; serverIndex++) {
                var serverId = gwssesAvailableSpace[serverIndex]._id;
                if (objectIdIndexOf(pipelineServers.Servers, serverId) !== -1
                    || gwssesAvailableSpace[serverIndex].Space - estimateSpaceChange(config, pipelineSizeMB) < 0
                    || gwsses[serverId].Request.Counts.Publish >= config.maxRequestCounts.Publish) {
                    continue;
                }
                upsertRequest(gwsses[serverId].Request, pipelineToServe, "Publish");
                gwssesAvailableSpace[serverIndex].Space -= estimateSpaceChange(config, pipelineSizeMB);
                numServersNeeded--;
                pipelinesHostsMap[pipelineToServe]--;
                pipelineServers.Servers.push(serverId);
                madeAllocation = true;
                return;
            }
        });
    } while (madeAllocation);
}

/**
 * Add requests to gwsses to remove pipelines that are excessively served 
 */
function removeExcessivelyServedPipelines (gwsses, gwssesAvailableSpace, config) {
    var bytesPerMB = 1024 * 1024;
    var pipelinesToRemove = db.Pipelines.find({ "Count": { $gt: config.minimumHosts } });
    var pipelinesProcRefs = db.Pipelines.distinct("_id", { "Count": { $gt: config.minimumHosts } });
    var pipelinesDetails = db.Resources.find({ "Type": "GeoSource", "ProcRef": { $in: pipelinesProcRefs } }, { "_id": 0, "ProcRef": 1, "DataSize": 1, "Metadata.SystemTags": 1 }).toArray();
    var pipelinesSizeMap = generatePipelinesMap("DataSize", pipelinesDetails);
    var pipelinesHostsMap = generatePipelinesHostsMap(pipelinesDetails, config.minimumHosts, config.slaHosts);
    var gwssesAvailableSpaceMap = {};
    for (var serverIndex = 0; serverIndex < gwssesAvailableSpace.length; serverIndex++) {
        gwssesAvailableSpaceMap[gwssesAvailableSpace[serverIndex]._id] = serverIndex;
    }

    pipelinesToRemove.forEach(function (pipelineServers) {
        var pipelineToRemove = pipelineServers._id;
        var numServersToRemove = pipelineServers.Servers.length - pipelinesHostsMap[pipelineToRemove];
        serversSpace = [];
        for (var serverIndex = 0; serverIndex < pipelineServers.Servers.length; serverIndex++) {
            serverSpaceIndex = gwssesAvailableSpaceMap[pipelineServers.Servers[serverIndex]];
            var gwssAvailableSpace = gwssesAvailableSpace[serverSpaceIndex];
            serversSpace.push({ "_id": gwssAvailableSpace._id, "Space": gwssAvailableSpace.Space });
        }
        // remove from the server with the least space first
        serversSpace.sort(function (a, b) {
            return a.Space - b.Space;
        })

        var pipelineSizeMB = pipelinesSizeMap[pipelineToRemove] / bytesPerMB;
        for (var serverIndex = 0; numServersToRemove > 0 && serverIndex < pipelineServers.Servers.length; serverIndex++) {
            var serverId = serversSpace[serverIndex]._id;
            var removedIndex = objectIdIndexOf(pipelineServers.Servers, serverId);
            if (gwsses[serverId].Request.Counts.Remove >= config.maxRequestCounts.Remove) {
                continue;
            }
            pipelineServers.Servers.splice(removedIndex, 1);
            upsertRequest(gwsses[serverId].Request, pipelineToRemove, "Remove");
            pipelineServers.Count--;
            gwssesAvailableSpace[gwssesAvailableSpaceMap[serverId]].Space += estimateSpaceChange(config, pipelineSizeMB);
            numServersToRemove--;
        }
    })
}

/**
 * Prepare a common request for all Publishing group gwsses
 */
function preparePublishingRequest(gwsses, activeServedProcRefs) {
    // just a stub implementation to always request everything for Publishing group (and keep any existing remove requests)
    var request = {
        PipelinesRequests: activeServedProcRefs.map(function(procRef) {
            return { "_t": "Operation", OperationType: "Publish", Parameters: { ProcRef: procRef } };
        }),
        Counts: {
            Publish: activeServedProcRefs.length,
            Remove: 0
        }
    };
    for (var gwssId in gwsses) {
        if (gwsses.hasOwnProperty(gwssId)) {
            var gwssRequest = gwsses[gwssId].Request;
            gwssRequest.PipelinesRequests = gwssRequest.PipelinesRequests.filter(function (request) {
                return request.OperationType === "Remove";
            });
            gwssRequest.PipelinesRequests = gwssRequest.PipelinesRequests.concat(request.PipelinesRequests);
            gwssRequest.Counts.Publish = request.Counts.Publish;
        }
    }
}

function updateGwssRequests (gwsses) {
    for (var gwssId in gwsses) {
        if (gwsses.hasOwnProperty(gwssId)) {
            db.Gwsses.update({ "_id": gwsses[gwssId]._id }, { $set: { "Request": gwsses[gwssId].Request } });
        }
    }
}


var config = {
    cutoffMilliseconds: 30 * 60 * 1000,
    minimumHosts: 3,
    maximumHosts: 6,
    maxRequestCounts: {
        Publish: 2,
        Remove: 2
    },
    spaceSafetyMargin: 2
};
config.slaHosts = {
    'SLA-High': config.maximumHosts
};

purgeInactiveServers(config);

var gwsses = getGwssesView();
var groupedGwsses = {
    Publishing: gwsses.filter(function (gwss) { return gwss.Group === "Publishing"; }),
    Processing: gwsses.filter(function (gwss) { return gwss.Group !== "Publishing"; })
};

var activeServedProcRefs = getActiveServedProcRefs();
generatePipelinesCollection(groupedGwsses.Processing, activeServedProcRefs);
initializeGwssRequestCounts(groupedGwsses.Processing);
var gwssesAvailableSpace = getGwssesAvailableSpace(groupedGwsses.Processing, config);

removeNonActiveServedPipelines(gwsses, activeServedProcRefs);

publishInsufficentlyServedPipelines(groupedGwsses.Processing, gwssesAvailableSpace, config);
removeExcessivelyServedPipelines(groupedGwsses.Processing, gwssesAvailableSpace, config);

preparePublishingRequest(groupedGwsses.Publishing, activeServedProcRefs);
updateGwssRequests(gwsses);