load("/home/Pyxis/scripts/analytics/analyticsFactory.js");

function guidFromBin(bin) {
	return bin.toCSUUID().substr(8,36);
}

function quote(str) {
	return '\"' + (str != null ? str.toString().replace(/"/g,'\\"').replace(/\r?\n|\r/g,' ') : '') + '\"';
}

function filterResources(resource) {
	return !resource.State || resource.State == "Active";
}

function filterVisibility(visibility, resource) {
	return resource.Metadata.Visibility === visibility;
}

function getSince(days) {
	days = Math.ceil(days);
        var currentDate = ISODate();
        var since = ISODate(currentDate.toISOString());
	since.setDate(since.getDate()-days);
	return since;
}

function resourceReport(type,af,days,format) {
	var since = getSince(days);
	var rs = af.resourcesOfType(type).toArray().filter(filterResources);
	var publicRs = rs.filter(filterVisibility.bind(this, "Public"));
	var nonDiscoverableRs = rs.filter(filterVisibility.bind(this, "NonDiscoverable"));
	var privateRs = rs.filter(filterVisibility.bind(this, "Private"));
	var rsSince = af.resourcesOfTypeCreatedSince(type,since).toArray().filter(filterResources);
	switch(format) {
		case 'txt':
			print(type + ': ' + rsSince.length + ', ' + rs.length + ', ' + publicRs.length + ', ' + nonDiscoverableRs.length + ', ' + privateRs.length);
			break;
		case 'csv':
			print('"' + type + '",' + rsSince.length + ',' + rs.length + ',' + publicRs.length + ',' + nonDiscoverableRs.length + ',' + privateRs.length);
			break;
	}
}

function bulletinStats() {
	try {	
		db.getMongo().setSlaveOk();
		var af = analyticsFactory();
		switch(format) {
			case 'txt':
				print('Resource type: Added the past ' + days + ' days, Total, Public, NonDiscoverable, Private');
				break;
			case 'csv':		
				print('"Resource type","Added the past ' + days + ' days","Total","Public","NonDiscoverable","Private"');
				break;
			default:
				print('Unsupported format');
				retun;
		}
		resourceReport('GeoSource',af,days,format);
		resourceReport('Map',af,days,format);
		resourceReport('Gallery',af,days,format);
	} catch (error) {
		print('Specify format and days using --eval');
		print(error);
	}
}

var urlBase = 'https://worldview.gallery';
bulletinStats()
