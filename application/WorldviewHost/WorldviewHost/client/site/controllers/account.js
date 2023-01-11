app.controller('worldviewAccountController', function ($scope, $http, $pyx, wvAlerts, user) {
    $scope.username = $pyx.user.username();
    $scope.profile = user;
    $scope.loadingUsage = true;
    $scope.storage = {};
    $scope.sending = false;

    $scope.account = function () {
        if ($scope.profile.Roles && $scope.profile.Roles.length) {
            return $scope.profile.Roles[0];
        }
        return $scope.username == 'Pyxis' ? "admin" : "beta";
    }();

    $pyx.user.quota().success(function (quota) {
        $scope.storage.quota = parseInt(quota) / (1024 * 1024); //MB
    });

    $pyx($scope.profile).geoSources().select('Id', 'ProcRef', 'DataSize', 'Metadata/Name').expand('Metadata').getAll().success(function (usage) {
        $scope.loadingUsage = false;
        $scope.usage = usage;
        $scope.storage.used = 0;
        angular.forEach($scope.usage, function (item) {
            $scope.storage.used += item.DataSize;
        });
        $scope.storage.used /= 1024 * 1024; //MB
        $scope.storage.usedPercent = 100 * ($scope.storage.used / $scope.storage.quota);
    }).error(function (error) {
        $scope.loadingUsage = false;
        $scope.loadingUsageError = error;
    });
    
    $scope.changePassword = false;
    $scope.changePasswordOpen = function () {
        $scope.changePassword = true;
        $scope.changePasswordValues = {
            oldPassword: "",
            newPassword: "",
            confirmPassword: "",
        }
    }

    $scope.changePasswordCommit = function () {
        var values = $scope.changePasswordValues;
        $pyx.user.changePassword(values.oldPassword, values.newPassword, values.confirmPassword)
            .success(function () {
                wvAlerts.success('Your password was successfully changed');
                $scope.changePassword = false;
            }).error(function (error) {
                wvAlerts.error('Changing your password failed with error: ' + error);
            });
    }

    $scope.changePasswordCancel = function () {
        $scope.changePassword = false;
    }

    $scope.sendConfirmationEmail = function () {
        $scope.sending = true;
        $pyx.gallery.sendConfirmationEmail()
            .success(function () {
                wvAlerts.success('Email Sent', $scope);
                $scope.sending = false;
            })
            .error(function () {
                wvAlerts.error('Sorry, we had a problem sending your request to our servers.', $scope);
                $scope.sending = false;
            });
    }

    $scope.togglingConsent = false;
    $scope.toggleConsent = function () {
        $scope.toggleConsent = true;
        $pyx.user.setConsent(!$scope.profile.PromotionConsent).success(function() {
            $scope.profile.PromotionConsent = !$scope.profile.PromotionConsent;
            $scope.toggleConsent = false;
        }).error(function () {
            wvAlerts.error('Sorry, we had a problem sending your request to our servers.', $scope);
            $scope.toggleConsent = false;
        });
    }
});