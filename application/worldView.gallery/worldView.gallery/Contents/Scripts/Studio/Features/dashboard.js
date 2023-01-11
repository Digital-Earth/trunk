app.service("featureDashboard", function ($pyx, $timeout, $pyxIntercom, worldViewStudioConfig) {

    function register($scope) {
        $scope.addTemplate('sections', '/template/feature-dashboard.html');

        $scope.addWidgetPopupMenu = {
            active: false
        };

        $scope.showAddWidgetMenu = function (item, field) {
            $scope.addWidgetPopupMenu = {
                active: true,
                item: item,
                field: field,
                widgets: []
            };

            if (field.FieldType === "Number") {
                $scope.addWidgetPopupMenu.widgets.push({
                    'Type': "Average",
                    'FieldName': field.Name,
                    'Title': field.Metadata.Name,
                    'Value': "Avg.",
                    'Tooltip': "tooltip.choose_widget.average"
                });

                $scope.addWidgetPopupMenu.widgets.push({
                    'Type': "Sum",
                    'FieldName': field.Name,
                    'Title': field.Metadata.Name,
                    'Value': "Sum",
                    'Tooltip': "tooltip.choose_widget.sum"
                });
            }

            $scope.addWidgetPopupMenu.widgets.push({
                'Type': "Min",
                'FieldName': field.Name,
                'Title': field.Metadata.Name,
                'Value': "Min",
                'Tooltip': "tooltip.choose_widget.min"
            });

            $scope.addWidgetPopupMenu.widgets.push({
                'Type': "Max",
                'FieldName': field.Name,
                'Title': field.Metadata.Name,
                'Value': "Max",
                'Tooltip': "tooltip.choose_widget.max"
            });

            if (field.Value) {
                $scope.addWidgetPopupMenu.widgets.push({
                    'Type': "Count",
                    'FieldValue': field.Value,
                    'Title': field.Value,
                    'Value': "Count",
                    'Tooltip': "tooltip.choose_widget.count.value"
                });
            }

            $scope.addWidgetPopupMenu.widgets.push({
                'Type': "Count",
                'FieldName': field.Name,
                'Title': item.Metadata.Name,
                'Value': "Count",
                'Tooltip': "tooltip.choose_widget.count.geosource"
            });

            $scope.addWidgetPopupMenu.widgets.push({
                'Type': "Hist",
                'FieldName': field.Name,
                'Title': field.Metadata.Name,
                'Value': "Graph",
                'Size': { 'rows': 2, 'cols': 2 },
                'Tooltip': "tooltip.choose_widget.distribution"
            });

            $scope.addWidgetPopupMenu.widgets.push({
                'Type': "Area",
                'Title': "Area",
                'Value': "Area",
                'Tooltip': "tooltip.choose_widget.area"
            });
        };

        $scope.dashboard = {
            minimized: true
        };

        $scope.quickDashboardPopup = false;
        $scope.quickDashboardPopupTimeout = undefined;

        $scope.popupDashboard = function () {
            if ($scope.quickDashboardPopupTimeout) {
                $timeout.cancel($scope.quickDashboardPopupTimeout);
            }
            $scope.quickDashboardPopup = true;
            $scope.quickDashboardPopupTimeout = $timeout(function () {
                $scope.quickDashboardPopup = false;
                $scope.quickDashboardPopupTimeout = undefined;
            }, worldViewStudioConfig.dashboard.popupTime);
        }

        $scope.addWidget = function (item, field, type, title, value) {
            $scope.addWidgetPopupMenu.active = false;
            $scope.currentMap.dashboard(0).addWidget(type, item, field.Name, title, value);
            field.OnDashboard = true;
            $scope.notifyLibraryChange();
            $scope.dashboard.minimized = false;

            var metadata = {
                item: item,
                name: field.Name,
                title: title,
                type: type,
                value: value
            };

            $pyxIntercom.track('add-widget', metadata);
        };

        $scope.removeWidget = function (dashboard, index) {
            dashboard.removeWidget(index);
            $scope.notifyLibraryChange();
        };

    };

    return {
        register: register
    };
});