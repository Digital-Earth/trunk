load("uuidhelpers.js")

function GuidFromCSUUID(id) {
	return id.toCSUUID().substring(8,44);	
}

function getPIICSV() {
	var accounts = db.AspNetUsers.find({},{ResourceId:1,UserName:1,Email:1,ProfileName:1,CrmId:1})

	print("UserId,UserName,Email,Joined,SolveLink,UserLink");
	accounts.forEach(function(doc) { 
		if(doc.ResourceId) {
			var crmLink = doc.CrmId == null ? '' : 'https://secure.solve360.com/contact/' + doc.CrmId;
			var userId = GuidFromCSUUID(doc.ResourceId);
			var userLink = 'https://worldview.gallery/User/' + userId;
			print(userId + ',' + doc.UserName + ',' + doc.Email + ',' + doc._id.getTimestamp() + ',' + crmLink + ',' + userLink);
		}
	});
}

db.getMongo().setSlaveOk();
getPIICSV();
