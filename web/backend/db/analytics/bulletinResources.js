load("/home/Pyxis/scripts/analytics/analyticsFactory.js");

function guidFromBin(bin) {
	return bin.toCSUUID().substr(8,36);
}

function quote(str) {
	return '\"' + (str != null ? str.toString().replace(/"/g,'\\"').replace(/\r?\n|\r/g,' ') : '') + '\"';
}

function filterPipelines(resource) {
        return !resource.State || resource.State == "Active";
}

function getSince(days) {
        days = Math.ceil(days);
        var currentDate = ISODate();
        var since = ISODate(currentDate.toISOString());
        since.setDate(since.getDate()-days);
        return since;
}

function galleryReport(af, owners, days) {
	var since = getSince(days);
	print('Created,Visibility,Name,URL,# Resources,Owner,Email,Contact,Description')
        var gs = af.resourcesOfTypeCreatedSince('Gallery',since).toArray();
        gs.forEach(function(g){
                var id = guidFromBin(g.Id);
		var owner = owners.filter(function(o) { return o.ResourceId && o.ResourceId.base64() == g.Metadata.User._id.base64(); })[0];
		var ownerName = owner.FirstName ? ' (' + owner.FirstName + ' ' + owner.LastName + ')' : '';
		var crmLink = owner.CrmId == null ? '' : 'https://secure.solve360.com/contact/' + owner.CrmId;
                print(quote(g.Metadata.Created) + ',' + quote(g.Metadata.Visibility) + ',' + quote(g.Metadata.Name) + ',' + urlBase + '/' + g.Metadata.Name + ',' + g.Resources.length + ',' + quote(g.Metadata.User.Name + ownerName) + ',' + quote(owner.Email) + ',' + quote(crmLink) + ',' + quote(g.Metadata.Description));
        });
}

function pipelineReport(type, af, owners, days) {
	var since = getSince(days);
        print('Created,Visibility,Name,URL,Owner,Email,Contact,Description')
        var gs = af.resourcesOfTypeCreatedSince(type,since).toArray().filter(filterPipelines);
        gs.forEach(function(g){
                var id = guidFromBin(g.Id);
                var owner = owners.filter(function(o) { return o.ResourceId && o.ResourceId.base64() == g.Metadata.User._id.base64(); })[0];
                var ownerName = owner.FirstName ? ' (' + owner.FirstName + ' ' + owner.LastName + ')' : '';
                var crmLink = owner.CrmId == null ? '' : 'https://secure.solve360.com/contact/' + owner.CrmId;
                print(quote(g.Metadata.Created) + ',' + quote(g.Metadata.Visibility) + ',' + quote(g.Metadata.Name) + ',' + urlBase + '/' + type + '/' + id + ',' + quote(g.Metadata.User.Name + ownerName) + ',' + quote(owner.Email) + ',' + quote(crmLink) + ',' + quote(g.Metadata.Description));
        });
}

function bulletinResources() {
	try {
		db.getMongo().setSlaveOk();
		var owners = db.AspNetUsers.find({}, {ResourceId:1,UserName:1,Email:1,FirstName:1,LastName:1,CrmId:1,_id:0}).toArray();
		var af = analyticsFactory();
		switch(type) {
			case 'Gallery':
				galleryReport(af,owners,days);
				break;
			case 'GeoSource':
				pipelineReport('GeoSource',af,owners,days);
				break;
			case 'Map':
		                pipelineReport('Map',af,owners,days);
				break;
			default:
				print('Unsupported type');
		}
	} catch (error) {
		print('Specify type and days using --eval');
                print(error);
	}
}

var urlBase = 'https://worldview.gallery';
bulletinResources()
