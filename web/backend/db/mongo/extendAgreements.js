load('/home/Pyxis/scripts/mongo/uuidhelpers.js');

function extendAgreements(extendTo) {
	db.Activities.update({Type:"Agreement"},{$set:{Expiration:ISODate(extendTo)}}, {multi:1})
}

extendAgreements(extendTo)
