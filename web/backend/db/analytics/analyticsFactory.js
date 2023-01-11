load("/home/Pyxis/scripts/analytics/uuidhelpers.js");

function analyticsFactory() {
		var gwssesCursor = db.Gwsses.find();
		var gwsses = {};
		var numServers = 0;
		gwssesCursor.forEach(function (gwss) {
			gwsses[gwss._id] = gwss;
			numServers++;
		});
        var pipelines = [];
        db.Resources.find({Type:"GeoSource"}).forEach(function (geoSource) {
		pipelines.push(geoSource);
	});
	var servers = [];
        for (var gwss in gwsses) {
    		if (gwsses.hasOwnProperty(gwss)) {
        		servers.push(gwsses[gwss]);
       		}
        }
	var pipelineServers = [];
	var pipelineServersMap = {};
        var webServiceTags = ["OGC", "WebService", "GeoServices"];
        var Statuses = {
            OK: { Name: "OK", Order: 3, Percent: 100 }, Working: { Name: "Working", Order: 2, Percent: 0},
            Waiting: { Name: "Waiting", Order: 1, Percent: 0 }, Bad: { Name: "Bad", Order: 0, Percent: 0 }
        };

	function numPipelines (server) {
            server.NumPipelines = server != null ? server.PipelineStatuses.length : 0;
            return server.NumPipelines;
        }

        function getPipelineName (procRef) {
            if (pipelines != undefined) {
                var pipelineIndex = findIndexOf(pipelines, function (p) { return p.ProcRef == procRef });
                if (pipelineIndex != -1) {
                    return pipelines[pipelineIndex].Name;
                } else {
                    return "[[Removed GeoSource]]";
                }
            }
            return "";
        }

        function pipelineStatus (pipeline) {
            if (pipeline.Status == "Downloading") {
                if (pipeline.OperationStatus == undefined || pipeline.OperationStatus.Progress == undefined) {
                    return "Preparing Download";
                }
            }
            return pipeline.Status;
        }

        var statusOrder = { Initializing: 0, Downloading: 1, Processing: 2, Publishing: 3, Published: 4, Removing: 5 }
        function sortPipelineServerArray (serverA, serverB) {
            return statusOrder[serverA.Status] > statusOrder[serverB.Status]
                || (statusOrder[serverA.Status] == statusOrder[serverB.Status] && serverA.Name > serverB.Name);
        }

        function setPipelineStatus (pipeline) {
            if (pipeline.Web) {
                if (pipeline.Servers.length === 0) {
                    pipeline.Status = Statuses.OK.Order; pipeline.StatusName = Statuses.OK.Name;
                }
                else if (pipeline.Servers.filter(function (s) { return s.Status === "Removing"; }).length === pipeline.Servers.length) {
                    pipeline.Status = Statuses.Working.Order; pipeline.StatusName = Statuses.Working.Name;
                }
                else {
                    pipeline.Status = Statuses.Bad.Order; pipeline.StatusName = Statuses.Bad.Name;
                }
                return;
            }
            if (pipeline.Active) {
                if (pipeline.Servers.length === 0 || pipeline.Servers.filter(function (s) { return s.Status === "Removing"; }).length === pipeline.Servers.length) {
                    pipeline.Status = Statuses.Bad.Order; pipeline.StatusName = Statuses.Bad.Name;
                }
                else if (pipeline.Servers.filter(function (s) { return s.Status === "Published"; }).length > 0) {
                    pipeline.Status = Statuses.OK.Order; pipeline.StatusName = Statuses.OK.Name;
                }
                else if (pipeline.Servers.filter(function (s) { return s.Status === "Publishing" || s.Status === "Processing"; }).length > 0) {
                    pipeline.Status = Statuses.Working.Order; pipeline.StatusName = Statuses.Working.Name;
                }
                else {
                    pipeline.Status = Statuses.Waiting.Order; pipeline.StatusName = Statuses.Waiting.Name;
                }
            }
            else {
                if (pipeline.Servers.length === 0) {
                    pipeline.Status = Statuses.OK.Order; pipeline.StatusName = Statuses.OK.Name;
                }
                else if (pipeline.Servers.filter(function (s) { return s.Status === "Removing"; }).length === pipeline.Servers.length) {
                    pipeline.Status = Statuses.Working.Order; pipeline.StatusName = Statuses.Working.Name;
                }
                else {
                    pipeline.Status = Statuses.Bad.Order; pipeline.StatusName = Statuses.Bad.Name;
                }
            }
        }

        function computeStatusPercentages () {
            var totalPipelines = pipelineServers.length;
            var counts = { OK: 0, Waiting: 0, Working: 0, Bad: 0 };
            for (var pipelineIndex = 0; pipelineIndex < pipelineServers.length; pipelineIndex++) {
                counts[pipelineServers[pipelineIndex].StatusName]++;
            }
            for (var status in counts) {
                if (counts.hasOwnProperty(status)) {
                    Statuses[status].Percent = counts[status] / (totalPipelines * 0.01);
                }
            }
        }

        function getStatusPercent (status) {
            return Statuses[status].Percent;
        }

        function constructPipelineServers () {
            for(var pipelineIndex = 0; pipelineIndex  < pipelines.length; pipelineIndex++) {
                var pipeline = pipelines[pipelineIndex];
                pipelineServersMap[pipeline.ProcRef] = {
                    Name: pipeline.Metadata.Name, ProcRef: pipeline.ProcRef, DataSize: pipeline.DataSize,
                    User: pipeline.Metadata.User.Name, Created: pipeline.Metadata.Created, SystemTags: pipeline.Metadata.SystemTags, 
                    Web: isWebServicePipeline(pipeline), Active: pipeline.State == "Active", Servers: [], Requests: [],
		    Id: pipeline.Id
                };
            }
            for(var serverIndex = 0; serverIndex < servers.length; serverIndex++) {
                var server = servers[serverIndex].Status;
		var serverId = servers[serverIndex]._id;
                for(var pipelineIndex = 0; pipelineIndex < server.PipelinesStatuses.length; pipelineIndex++) {
                    var pipelineStatus = server.PipelinesStatuses[pipelineIndex];
                    if (pipelineServersMap[pipelineStatus.ProcRef] !== undefined
                        && pipelineServersMap[pipelineStatus.ProcRef]["Servers"].filter(function (s) { return s.Name == server.Name; }).length == 0) {
                        pipelineServersMap[pipelineStatus.ProcRef]["Servers"].push({ Name: server.Name, Status: pipelineStatus.StatusCode, NodeID: serverId });
                    }
                }
            }
            var operationToStatus = { Publish: "Published", Remove: "Removing" };
            for (var serverIndex = 0; serverIndex < servers.length; serverIndex++) {
                var server = servers[serverIndex];
                if (server.Request === undefined) {
                    continue;
                }
                for (var pipelineIndex = 0; pipelineIndex < server.Request.PipelinesRequests.length; pipelineIndex++) {
                    var pipelineRequest = server.Request.PipelinesRequests[pipelineIndex];
                    var procRef = pipelineRequest.Parameters["ProcRef"];
                    if (pipelineServersMap[procRef] !== undefined
                        && pipelineServersMap[procRef]["Requests"].filter(function (s) { return s.Name == server.Name; }).length == 0) {
                        pipelineServersMap[procRef]["Requests"].push({ Name: server.Status.Name, Status: operationToStatus[pipelineRequest.OperationType], NodeID: server.Id });
                    }
                }
            }
            for (var procRef in pipelineServersMap) {
                if (pipelineServersMap.hasOwnProperty(procRef)) {
                    setPipelineStatus(pipelineServersMap[procRef]);
                    pipelineServersMap[procRef]["Servers"].sort(sortPipelineServerArray);
                    pipelineServersMap[procRef]["Requests"].sort(sortPipelineServerArray);
                    pipelineServers.push(pipelineServersMap[procRef]);
                }
            }
            computeStatusPercentages();
        }
        
        function isWebServicePipeline (pipeline) {
            for (var tagIndex = 0; tagIndex < pipeline.Metadata.SystemTags.length; tagIndex++) {
                if (webServiceTags.indexOf(pipeline.Metadata.SystemTags[tagIndex]) !== -1) {
                    return true;
                }
            }
            return false;
        }

	function pipelinesOfStatus(statusName) {
		var pipelines = [];
		for (var procRef in pipelineServersMap) {
			if (pipelineServersMap.hasOwnProperty(procRef)) {
				var pipeline = pipelineServersMap[procRef];
				if(pipeline.StatusName == statusName) {
					pipelines.push(pipeline);
				}
			}
		}
		return pipelines;
	}

	constructPipelineServers();
	
	return {
                dbStats: function() {
                        return db.stats();
                },
		totalAccounts: function() {
                        return db.AspNetUsers.count();
                },
		resourcesOfType: function(type) {
                        return db.Resources.find({Type:type});
                },
		resourcesCreatedSince: function(sinceDate) {
			return db.Resources.find({"Metadata.Created":{$gte:sinceDate}},{Id:1,Metadata:1,Type:1,Resources:1,State:1,_id:0});
		},
                resourcesOfTypeCreatedSince: function(type,sinceDate) {
                        return db.Resources.find({"Metadata.Created":{$gte:sinceDate},"Type":type},{Id:1,Metadata:1,Type:1,Resources:1,State:1,_id:0});
                },
		resourcesUpdatedSince: function(sinceDate) {
                        return db.ResourcesArchive.find({"Metadata.Updated":{$gte:sinceDate}},{Id:1,Metadata:1,Type:1,Resources:1,State:1,_id:0});
                },
                resourcesOfTypeUpdatedSince: function(type,sinceDate) {
                        return db.ResourcesArchive.find({"Metadata.Updated":{$gte:sinceDate},"Type":type},{Id:1,Metadata:1,Type:1,Resources:1,State:1,_id:0});
                },
		totalResourcesOfType: function(type) {
                        return this.resourcesOfType(type).count();
                },
                activeGeoSources: function() {
                        return db.Resources.find({Type:"GeoSource", State:"Active"}).count();
                },
                removedGeoSources: function() {
                        return db.Resources.find({Type:"GeoSource", State:"Removed"}).count();
                },
		geoSourceSizes: function() {
			var sizes = [];
			db.Resources.aggregate([{$match:{Type:"GeoSource"}},{$group:{_id:{state:"$State"}, dataSize:{"$sum":"$DataSize"}}}]).forEach(function(size) {
				sizes.push({"State":size._id.state, "DataSize": size.dataSize.toNumber()});
			});
			return sizes;
		},
                brokenGeoSources: function() {
                        return db.Resources.find({Type:"GeoSource", State:"Broken"}).count();
                },
                totalGwsses: function() {
                        return numServers;
                },
		gwssesByName: function() {
			return servers.map(function (gwss) {
				return gwss.Status.Name; 
			});
		},
		pipelinePublishingStatuses: function() {
			return Statuses; 
		},
		pipelinePublishingStatusDetails: function(statusName) {
			return pipelinesOfStatus(statusName);
		},
		resourceDistribution: function() {
			return db.Resources.aggregate([{$group:{_id:"$Type",Type:{$first:"$Type"},count:{$sum:1}}}, {$sort:{count:-1}}, {$project:{_id:0,Type:1,count:1}}]);
		},
		resourceDocSizeDistribution: function() {
			var sizes=[]; 
			db.Resources.find().forEach(function(doc){ sizes.push({_id:doc._id, Type: doc.Type, size: Object.bsonsize(doc)})});
			sizes = sizes.sort(function(a,b) {return a.size-b.size;});
			return sizes;
		},
		resourceArchiveDocSizeDistribution: function() {
                        var sizes=[];
                        db.ResourcesArchive.find().forEach(function(doc){ sizes.push({_id:doc._id, Type: doc.Type, size: Object.bsonsize(doc)})});
                        sizes = sizes.sort(function(a,b) {return a.size-b.size;});
                        return sizes;
		}
        }
}

function printPipelines(pipelines) {
	pipelines.sort(function(a,b) { return a.Created - b.Created; });
	for(var i = 0; i < pipelines.length; i++) {
		print(pipelines[i].Created  + " " + pipelines[i].ProcRef + " " + pipelines[i].User + " \"" + pipelines[i].Name + "\"");
	}
}

function commaSeparateNumber(val){
        while (/(\d+)(\d{3})/.test(val.toString())){
                val = val.toString().replace(/(\d+)(\d{3})/, '$1'+','+'$2');
        }
        return val;
}

