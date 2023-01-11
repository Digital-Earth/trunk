load("/home/Pyxis/scripts/analytics/adminAnalyticsFactory.js");

function adminAnalytics(){
	var aaf = adminAnalyticsFactory();
	var replStatus = aaf.replSetStatus();
	var replString = replStatus.set + " replication set health - ";
	for(var i = 0; i < replStatus.members.length; i++) {
		var member = replStatus.members[i];
		replString += member.name + " (" + member.stateStr + "): " + (member.health==1 ? "Healthy" : "<span style='color:red'>Unhealthy</span>") + ", ";
	}
	print(replString);
}

adminAnalytics()
