load("/home/Pyxis/scripts/analytics/analyticsFactory.js");

function dayDiff(first, second) {
    return Math.round((first-second)/(1000*60*60*24));
}

function ResourceCount() {
	this.GeoSource=0;this.Pipeline=0;this.File=0;this.Map=0;this.Gallery=0;this.User=0;this.Group=0;this.License=0;this.Product=0;
}

var charts = {
	Resource: function resourceChart(af, days) {
		days = Math.ceil(days);
		var currentDate = ISODate();
		var since = ISODate(currentDate.toISOString());
		since.setDate(since.getDate()-days);
		var counts = [];
		for(var i=0; i < days; i++) {
			counts[i] = new ResourceCount();
		}
	        af.resourcesCreatedSince(since).forEach(function(r){
        		dayBin = dayDiff(currentDate,r.Metadata.Created);
			if(dayBin < days) {
				counts[dayBin][r.Type]++;
			}
		});
		print('Date,GeoSource,Pipeline,File,Map,Gallery,User,Group,License,Product');
		for(var i=days-1; i>=0; i--) {
			since.setDate(since.getDate()+1);
			print(since.toDateString()+","+counts[i].GeoSource+","+counts[i].Pipeline+","+counts[i].File+","+counts[i].Map+","+counts[i].Gallery+","+counts[i].User+","+counts[i].Group+","+counts[i].License+","+counts[i].Product);
		}
	},
	Updates: function resourceUpdatesChart(af, days, includeCreates) {
                days = Math.ceil(days);
                var currentDate = ISODate();
                var since = ISODate(currentDate.toISOString());
                since.setDate(since.getDate()-days);
                var counts = [];
                for(var i=0; i < days; i++) {
                        counts[i] = new ResourceCount();
                }
                af.resourcesUpdatedSince(since).forEach(function(r){
			if(!includeCreates && r.Metadata.Created.getTime() == r.Metadata.Updated.getTime()) {
				return;
			}
                        dayBin = dayDiff(currentDate,r.Metadata.Updated);
                        if(dayBin < days) {
                                counts[dayBin][r.Type]++;
                        }
                });
                print('Date,GeoSource,Pipeline,File,Map,Gallery,User,Group,License,Product');
                for(var i=days-1; i>=0; i--) {
                        since.setDate(since.getDate()+1);
                        print(since.toDateString()+","+counts[i].GeoSource+","+counts[i].Pipeline+","+counts[i].File+","+counts[i].Map+","+counts[i].Gallery+","+counts[i].User+","+counts[i].Group+","+counts[i].License+","+counts[i].Product);
                }
        },
	DocSize: function resourceDocSizeChart(af, bins, archive) {
                var counts = [];
                for(var i=0; i < bins+1; i++) {
                        counts[i] = new ResourceCount();
                }
		var sizes;
		if(!archive) {
                	sizes = af.resourceDocSizeDistribution();
		} else {
			sizes = af.resourceArchiveDocSizeDistribution();
		}
         	var minSize = sizes[0].size;
		var maxSize = sizes[sizes.length-1].size;
		for(var i=0; i<sizes.length; i++) {
			var resource = sizes[i];
			var bin = Math.floor(Math.log(1 + resource.size - minSize)/Math.log(1 + maxSize - minSize) * bins);
			counts[bin][resource.Type]++;
		}       
                print('Size,GeoSource,Pipeline,File,Map,Gallery,User,Group,License,Product');
                for(var i=0; i<bins+1; i++) {
                        // inexact bin boundary size calculation
			var size = minSize + Math.exp(i * Math.log(1 + maxSize-minSize)/bins)-1;
			var label = Math.floor(size) + "B";
			if(size >= 1024) {
				label = Math.floor(size / 1024) + "K";
			}
                        print(label+","+counts[i].GeoSource+","+counts[i].Pipeline+","+counts[i].File+","+counts[i].Map+","+counts[i].Gallery+","+counts[i].User+","+counts[i].Group+","+counts[i].License+","+counts[i].Product);
                }
	},
	DocSizeArchive: function resourceArchiveDocSizeChart(af, bins) {
		this.DocSize(af, bins, true) 
	}
};

function resourceCharts() {
	try {
        	db.getMongo().setSlaveOk();
	        var af = analyticsFactory();
		charts[chartType](af,bins);
	} catch (error) {
		print('Specify chartType and bins using --eval to generate the desired chart data. Valid types are: Resource, Updates, DocSize, DocSizeArchive');
		print(error)
		return;
	}
}

resourceCharts();
