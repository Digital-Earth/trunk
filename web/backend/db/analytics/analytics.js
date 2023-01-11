load("/home/Pyxis/scripts/analytics/analyticsFactory.js");

function analytics (){
	db.getMongo().setSlaveOk();
	var af = analyticsFactory();
	var dbStats = af.dbStats();
        print(commaSeparateNumber(dbStats.dataSize) + "B dataSize, " + commaSeparateNumber(dbStats.indexSize) + "B indexSize, " + commaSeparateNumber(dbStats.storageSize) + "B storageSize");
        print(commaSeparateNumber(dbStats.objects) + " objects, " + Math.floor(dbStats.avgObjSize) + "B avgObjSize, " + commaSeparateNumber(dbStats.collections) + " collections");
	print(af.totalAccounts() + " Accounts (" + af.totalResourcesOfType("User") + " Users)");
	print(af.totalResourcesOfType("Gallery") + " Galleries");
	print(af.totalResourcesOfType("GeoSource") + " GeoSources (" + af.activeGeoSources() + " active, " + af.removedGeoSources() + " removed, " + af.brokenGeoSources() + " broken)");
	var sizes = af.geoSourceSizes();
	var sizeString = "";
	var totalSize = 0;
	for(var i = 0; i < sizes.length; i++) {
		totalSize += sizes[i].DataSize;
		sizeString += "\"" + sizes[i].State + "\" " + commaSeparateNumber(sizes[i].DataSize) + "B, ";
	}
	sizeString = commaSeparateNumber(totalSize) + "B total DataSize (" + sizeString.substring(0,sizeString.length-2) + ")";
	print(sizeString);
	var gwsses = af.totalGwsses();
	var expectedGwsses = 6;
	gwssAlert = gwsses < expectedGwsses;
	gwssAlertOpen = gwssAlert ? "<span style='color:red'>" : "";
	gwssAlertClose = gwssAlert ? "</span>" : ""; 
	print(gwssAlertOpen + gwsses + gwssAlertClose + " Gwsses (at time of reporting)");
	var statusString = "Publishing pipeline statuses: ";
	var statuses = af.pipelinePublishingStatuses();
	for(var stat in statuses) {
		if (statuses[stat].Name === 'OK') {
			statusString += "<b>";
		}
		statusString += statuses[stat].Name;
		if (statuses[stat].Name === 'OK') {
			statusString += "</b>";
		}
		statusString += " ";
		if (statuses[stat].Name === 'OK' && statuses[stat].Percent.toString().substring(0,3) !== '100') {
                        statusString += "<span style='color:red'>";
                }
		statusString += statuses[stat].Percent.toString().substring(0,4) + "%";
		if (statuses[stat].Name === 'OK' && statuses[stat].Percent.toString().substring(0,3) !== '100') {
                        statusString += "</span>";
                }
		statusString += ", ";
	}
	print(statusString.substring(0, statusString.length-2));
	if(statuses["Bad"].Percent > 0) {
		var pipelines = af.pipelinePublishingStatusDetails("Bad");
		print("\nBad Pipelines (" + pipelines.length + ")\n=================")
		printPipelines(pipelines);
	}
	if(statuses["Waiting"].Percent > 0) {
                var pipelines = af.pipelinePublishingStatusDetails("Waiting");
		print("\nWaiting Pipelines (" + pipelines.length + ")\n=================")
                printPipelines(pipelines);
        }
}

analytics()
