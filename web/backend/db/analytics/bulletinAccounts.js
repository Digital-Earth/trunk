load("/home/Pyxis/scripts/analytics/uuidhelpers.js")

function dateString(d) {
	return d ? d.toISOString() : '';
}

function safeString(v) {
	return v ? v : '';
}

function loginProviderString(p) {
	return p ? p : 'Local';
}

function bulletinAccounts() {
	db.getMongo().setSlaveOk();
	var accounts = db.AspNetUsers.find({},{UserName:1,Email:1,FirstName:1,LastName:1,_id:1,LastLogin:1,CrmId:1,Roles:1,Country:1,City:1,ExternalLoginProvider:1}).sort({_id:-1}).toArray();
	print("Joined,Roles,Email,First Name, Last Name,User Name,Last Login,CrmId,Country,City,Login Provider");
	accounts.forEach(function(doc) { print(dateString(doc._id.getTimestamp()) + ',' + doc.Roles[0] + ',' + doc.Email + ',' + safeString(doc.FirstName) + ',' + safeString(doc.LastName) + ',' + doc.UserName + ',' + dateString(doc.LastLogin) + ',' + safeString(doc.CrmId) + ',' + safeString(doc.Country) + ',' + safeString(doc.City) + ',' + loginProviderString(doc.ExternalLoginProvider)); })
}

bulletinAccounts();
