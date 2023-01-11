using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Web;
using LicenseServer.App_Utilities.Serializers;
using LicenseServer.DTOs;
using LicenseServer.Models.Mongo;
using MongoDB.Bson;
using MongoDB.Bson.Serialization;
using MongoDB.Bson.Serialization.Serializers;
using Newtonsoft.Json.Linq;
using Pyxis.Contract.Operations;
using Pyxis.Contract.Publishing;
using Pyxis.Contract.Services.GeoWebStreamService;
using File = LicenseServer.Models.Mongo.File;
using Gallery = LicenseServer.Models.Mongo.Gallery;
using GeoSource = LicenseServer.Models.Mongo.GeoSource;
using License = LicenseServer.Models.Mongo.License;
using Map = LicenseServer.Models.Mongo.Map;
using Pipeline = LicenseServer.Models.Mongo.Pipeline;
using Product = LicenseServer.Models.Mongo.Product;
using User = LicenseServer.Models.Mongo.User;
using Group = LicenseServer.Models.Mongo.Group;
using MultiDomainGeoSource = LicenseServer.Models.Mongo.MultiDomainGeoSource;

namespace LicenseServer
{
    public static class MongoConfig
    {
        public static void RegisterClassMaps()
        {
            // Register custom serializers
            BsonSerializer.RegisterSerializer(typeof(JObject), new CustomJObjectSerializer());
            BsonSerializer.RegisterSerializer(typeof(Color), new CustomColorSerializer());

            // Register types for deserialization
            BsonClassMap.RegisterClassMap<PyxisIdentityUser>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
            });

            BsonClassMap.RegisterClassMap<ServerStatus>();
            BsonClassMap.RegisterClassMap<NetworkStatus>();
            BsonClassMap.RegisterClassMap<Operation>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.GetMemberMap(c => c.OperationType).SetSerializer(new EnumSerializer<OperationType>(BsonType.String));
            });
            BsonClassMap.RegisterClassMap<PipelineStatusImpl>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.GetMemberMap(c => c.StatusCode).SetSerializer(new EnumSerializer<PipelineStatusCode>(BsonType.String));
            });
            BsonClassMap.RegisterClassMap<OperationStatus>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.GetMemberMap(c => c.StatusCode).SetSerializer(new EnumSerializer<OperationStatusCode>(BsonType.String));
            });

            // Set class BsonIds, Ignore extra elements for rolling updates and enforce writing enums as strings
            BsonClassMap.RegisterClassMap<ResourceReference>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.GetMemberMap(c => c.Type).SetSerializer(new EnumSerializer<ResourceType>(BsonType.String));
            });

            BsonClassMap.RegisterClassMap<LicenseTerms>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
            });

            BsonClassMap.RegisterClassMap<LicenseTrialLimitations>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
            });

            BsonClassMap.RegisterClassMap<LicenseExportOptions>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
            });

            BsonClassMap.RegisterClassMap<LicenseReference>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.GetMemberMap(c => c.LicenseType).SetSerializer(new EnumSerializer<LicenseType>(BsonType.String));
            });

            BsonClassMap.RegisterClassMap<VersionedId>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
            });

            BsonClassMap.RegisterClassMap<ResourceGrouping>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.SetIdMember(cm.GetMemberMap(c => c.Name));
            });

            BsonClassMap.RegisterClassMap<Resource>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.SetIdMember(null); // Force implicit Mongo _id
                cm.GetMemberMap(c => c.Id).SetElementName("Id");
                cm.GetMemberMap(c => c.Type).SetSerializer(new EnumSerializer<ResourceType>(BsonType.String));
                cm.SetIsRootClass(true);
                cm.SetDiscriminatorIsRequired(true);
            });

            BsonClassMap.RegisterClassMap<Provider>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.GetMemberMap(c => c.Type).SetSerializer(new EnumSerializer<ResourceType>(BsonType.String));
            });

            BsonClassMap.RegisterClassMap<UserInfo>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
            });

            BsonClassMap.RegisterClassMap<GroupInfo>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
            });

            BsonClassMap.RegisterClassMap<GroupPermissionInfo>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.GetMemberMap(c => c.Permission).SetSerializer(new EnumSerializer<GroupPermission>(BsonType.String));
            });

            BsonClassMap.RegisterClassMap<ExternalUrl>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.GetMemberMap(c => c.Type).SetSerializer(new EnumSerializer<ExternalUrlType>(BsonType.String));
            });

            BsonClassMap.RegisterClassMap<SimpleMetadata>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
            });

            BsonClassMap.RegisterClassMap<ImmutableMetadata>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.GetMemberMap(c => c.Visibility).SetSerializer(new NullableSerializer<VisibilityType>(new EnumSerializer<VisibilityType>(BsonType.String)));
            });

            BsonClassMap.RegisterClassMap<Metadata>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
            });
            
            BsonClassMap.RegisterClassMap<Pyxis.Contract.Publishing.User>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.GetMemberMap(c => c.State).SetSerializer(new NullableSerializer<UserState>(new EnumSerializer<UserState>(BsonType.String)));
                cm.GetMemberMap(c => c.UserType).SetSerializer(new NullableSerializer<UserType>(new EnumSerializer<UserType>(BsonType.String)));
            });

            BsonClassMap.RegisterClassMap<User>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.SetDiscriminatorIsRequired(true);
                cm.SetDiscriminator("LSUser");
            });

            BsonClassMap.RegisterClassMap<Pyxis.Contract.Publishing.Group>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
            });

            BsonClassMap.RegisterClassMap<Group>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.SetDiscriminatorIsRequired(true);
                cm.SetDiscriminator("LSGroup");
            });

            BsonClassMap.RegisterClassMap<PipelineSpecification.FieldUnit>(cm =>
            {
                cm.AutoMap();
            });

            BsonClassMap.RegisterClassMap<PipelineSpecification.FieldSpecification>(cm =>
            {
                cm.AutoMap();
                cm.GetMemberMap(c => c.FieldType).SetSerializer(new EnumSerializer<PipelineSpecification.FieldType>(BsonType.String));
            });

            BsonClassMap.RegisterClassMap<PipelineSpecification>(cm =>
            {
                cm.AutoMap();
                cm.GetMemberMap(c => c.OutputType).SetSerializer(new NullableSerializer<PipelineSpecification.PipelineOutputType>(new EnumSerializer<PipelineSpecification.PipelineOutputType>(BsonType.String)));
            });

            BsonClassMap.RegisterClassMap<Pyxis.Contract.Publishing.Pipeline>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
            });

            BsonClassMap.RegisterClassMap<Pipeline>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.SetDiscriminatorIsRequired(true);
                cm.SetDiscriminator("LSPipeline");
            });
            
            BsonClassMap.RegisterClassMap<Domain>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.GetMemberMap(c => c.Type).SetSerializer(new EnumSerializer<PipelineSpecification.FieldType>(BsonType.String));
            });

            BsonClassMap.RegisterClassMap<Pyxis.Contract.Publishing.GeoSource>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.GetMemberMap(c => c.State).SetSerializer(new NullableSerializer<PipelineDefinitionState>(new EnumSerializer<PipelineDefinitionState>(BsonType.String)));
            });

            BsonClassMap.RegisterClassMap<GeoSource>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.SetDiscriminatorIsRequired(true);
                cm.SetDiscriminator("LSGeoSource");
            });

            BsonClassMap.RegisterClassMap<Pyxis.Contract.Publishing.MultiDomainGeoSource>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
            });

            BsonClassMap.RegisterClassMap<MultiDomainGeoSource>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.SetDiscriminatorIsRequired(true);
                cm.SetDiscriminator("LSMDGeoSource");
            });

            BsonClassMap.RegisterClassMap<Pyxis.Contract.Publishing.Gallery>(cm =>
            {
                cm.AutoMap();
                cm.GetMemberMap(c => c.Admin).SetIgnoreIfNull(true);
                cm.SetIgnoreExtraElements(true);
            });

            BsonClassMap.RegisterClassMap<Gallery>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.SetDiscriminatorIsRequired(true);
                cm.SetDiscriminator("LSGallery");
            });

            BsonClassMap.RegisterClassMap<Pyxis.Contract.Publishing.License>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.GetMemberMap(c => c.LicenseType).SetSerializer(new NullableSerializer<LicenseType>(new EnumSerializer<LicenseType>(BsonType.String)));
                cm.GetMemberMap(c => c.PublishingType).SetSerializer(new NullableSerializer<PublishingType>(new EnumSerializer<PublishingType>(BsonType.String)));
            });

            BsonClassMap.RegisterClassMap<License>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.SetDiscriminatorIsRequired(true);
                cm.SetDiscriminator("LSLicense");
            });

            BsonClassMap.RegisterClassMap<Pyxis.Contract.Publishing.File>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
            });

            BsonClassMap.RegisterClassMap<File>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.SetDiscriminatorIsRequired(true);
                cm.SetDiscriminator("LSFile");
            });

            BsonClassMap.RegisterClassMap<Palette.Step>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
            });
            
            BsonClassMap.RegisterClassMap<Palette>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
            });

            BsonClassMap.RegisterClassMap<StyleEffect>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
            });

            BsonClassMap.RegisterClassMap<FieldStyle>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.SetIsRootClass(true);
                cm.SetDiscriminatorIsRequired(true);
                cm.GetMemberMap(c => c.Style).SetSerializer(new EnumSerializer<FieldStyleOptions>(BsonType.String));
            });

            BsonClassMap.RegisterClassMap<IconFieldStyle>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.SetDiscriminatorIsRequired(true);
            });

            BsonClassMap.RegisterClassMap<Style>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
            });

            BsonClassMap.RegisterClassMap<Dashboard.StyledSelection>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
            });

            BsonClassMap.RegisterClassMap<Dashboard.Widget>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
            });

            BsonClassMap.RegisterClassMap<Dashboard>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
            });

            BsonClassMap.RegisterClassMap<Camera>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
            });

            BsonClassMap.RegisterClassMap<Pyxis.Contract.Publishing.Map.Item>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
            });

            BsonClassMap.RegisterClassMap<Pyxis.Contract.Publishing.Map.Group>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
            });

            BsonClassMap.RegisterClassMap<Pyxis.Contract.Publishing.Map>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.GetMemberMap(c => c.State).SetSerializer(new NullableSerializer<PipelineDefinitionState>(new EnumSerializer<PipelineDefinitionState>(BsonType.String)));
            });

            BsonClassMap.RegisterClassMap<Map>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.SetDiscriminatorIsRequired(true);
                cm.SetDiscriminator("LSMap");
            });

            BsonClassMap.RegisterClassMap<Pyxis.Contract.Publishing.Product>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.GetMemberMap(c => c.ProductType).SetSerializer(new NullableSerializer<ProductType>(new EnumSerializer<ProductType>(BsonType.String)));
                cm.GetMemberMap(c => c.TransferType).SetSerializer(new NullableSerializer<TransferType>(new EnumSerializer<TransferType>(BsonType.String)));
            });

            BsonClassMap.RegisterClassMap<Product>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.SetDiscriminatorIsRequired(true);
                cm.SetDiscriminator("LSProduct");
            });

            BsonClassMap.RegisterClassMap<Activity>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.SetIdMember(cm.GetMemberMap(c => c.Id));
                cm.GetMemberMap(c => c.Id).SetIgnoreIfDefault(true);
                cm.GetMemberMap(c => c.Type).SetSerializer(new EnumSerializer<ActivityType>(BsonType.String));
                cm.SetIsRootClass(true);
                cm.SetDiscriminatorIsRequired(true);
            });

            BsonClassMap.RegisterClassMap<ResourceActivity>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.SetDiscriminatorIsRequired(true);
            });

            BsonClassMap.RegisterClassMap<Comment>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.SetDiscriminatorIsRequired(true);
            });
            
            BsonClassMap.RegisterClassMap<IndividualComment>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
            });

            BsonClassMap.RegisterClassMap<AggregateComment>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
            });

            BsonClassMap.RegisterClassMap<Rating>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.SetDiscriminatorIsRequired(true);
            });

            BsonClassMap.RegisterClassMap<AggregateRatings>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
            });

            BsonClassMap.RegisterClassMap<Agreement>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.GetMemberMap(c => c.Decision).SetSerializer(new EnumSerializer<DecisionType>(BsonType.String));
                cm.GetMemberMap(c => c.LicenseType).SetSerializer(new EnumSerializer<LicenseType>(BsonType.String));
                cm.SetDiscriminatorIsRequired(true);
            });

            BsonClassMap.RegisterClassMap<MongoDBEntities.MongoKeyValuePair>(cm =>
            {
                cm.AutoMap();
                cm.SetIdMember(cm.GetMemberMap(c => c.Key));
            });

            BsonClassMap.RegisterClassMap<Gwss>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
                cm.SetIdMember(cm.GetMemberMap(c => c.Id));
            });

            BsonClassMap.RegisterClassMap<LsStatus>(cm =>
            {
                cm.AutoMap();
                cm.SetIgnoreExtraElements(true);
            });

        }
    }
}