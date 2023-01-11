load("/home/Pyxis/scripts/analytics/analyticsFactory.js");

function guidFromBin(bin) {
	return bin.toCSUUID().substr(8,36);
}

function quote(str) {
	return '\"' + (str != null ? str.toString().replace(/"/g,'\\"').replace(/\r?\n|\r/g,' ') : '') + '\"';
}

function galleryReport(af) {
	print('Name,URL,Id,Visibility,# Resources,Owner,Tags,Description')
	af.resourcesOfType('Gallery').forEach(function(g){
		var id = guidFromBin(g.Id);
		print(quote(g.Metadata.Name) + ',' + urlBase + '/Gallery/' + id + ',' + id + ',' + g.Metadata.Visibility + ',' + g.Resources.length + ',' + quote(g.Metadata.User.Name) + ',' + quote(g.Metadata.Tags.toString()) + ',' + quote(g.Metadata.Description)); 
	});
}

function geoSourceReport(af) {
	print('Name,URL,Id,Visibility,State,Provider,Owner,Tags,Description')
        af.resourcesOfType('GeoSource').forEach(function(g){
                var id = guidFromBin(g.Id);
                print(quote(g.Metadata.Name) + ',' + urlBase + '/GeoSource/' + id + ',' + id + ',' + g.Metadata.Visibility + ',' + g.State + ',' + (g.Metadata.Providers[0] ? g.Metadata.Providers[0].Name : '') + ',' + quote(g.Metadata.User.Name) + ',' + quote(g.Metadata.Tags.toString()) + ',' + quote(g.Metadata.Description));
        });
}

function resourceReport() {
	db.getMongo().setSlaveOk();
	var af = analyticsFactory();
	galleryReport(af);
	geoSourceReport(af);
}

var urlBase = 'https://worldview.gallery';
resourceReport()
