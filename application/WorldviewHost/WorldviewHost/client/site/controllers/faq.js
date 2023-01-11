app.controller('worldviewFaqController', function ($scope, $pyx, $location) {
    $scope.faqs = [
        {
            "heading": "Account Info",
            "qaPairs": [
                {
                    "question": "Does my account cost anything to setup?",
                    "answer": "No. To create your account costs you nothing. Our objective is to enable everyone to have the ability to participate in the WorldView.Gallery to publish your own GeoSources and location-based data."
                }
            ]
        },
        {
            "heading": "Searching and Importing",
            "qaPairs": [
                {
                    "question": "How long should it take to load a GeoSource?",
                    "answer": "Once the file is opened or dropped into WorldView Studio it must be processed into the discrete global grid.  Most data files will take a few seconds before you will see the results.  Larger and more complex data can take several minutes – please be patient."
                }
            ]
        },
        {
            "heading": "Licensing and Other Restrictions",
            "qaPairs": [
                {
                    "question": "Is there a limit to how many Maps or GeoSources I can use?",
                    "answer": "No. You can import and use as many GeoSources and Maps as you like."
                },
                {
                    "question": "Is there a limit to how many Maps or GeoSources I can share?",
                    "answer": "Sharing a GeoSource involves publishing it, which requires that the data be loaded on a server. While we would like to give everyone an unlimited amount of storage for publishing data, unfortunately the costs associated with this storage make this untenable. For this reason we have decided to allow each user account 1 GB of free storage. You cannot publish more than that without upgrading to a paid account."
                }
            ]
        },
        {
            "heading": "Navigating",
            "qaPairs": [
                {
                    "question": "Why does the image on the Globe sometimes change to a grey background with a camera icon?",
                    "answer": "Imagery data has a resolution associated with it based on the device used to capture the image. This resolution determines the maximum magnification level at which the image can be viewed. As you zoom in on the Globe, you are increasing the magnification of the image. Eventually you can exceed the magnification at which the image can be displayed to an acceptable quality (as defined by the image properties). When that happens, WorldView Studio stops displaying the image and instead displays the grey background with the camera icon as an indication that the image resolution has been exceeded."
                },
                {
                    "question": "Why does the 'Go to your location' button sometimes go to a location other than where I am?",
                    "answer": "The 'Go to your location' functionality is not intended to be precise, rather it is an approximation based on your IP address, and is provided as a convenience to quickly reorient the Globe to your general vicinity. It will use the address of the Internet provider through which your computer is connected at the time. Usually this results in a close approximation to your actual location, but occasionally the Internet provider can be located a significant distance from your location."
                },
                {
                    "question": "Why are some icons bigger than others, and why do they disappear when I zoom in?",
                    "answer": "As you zoom out, the world shrinks--objects get smaller and closer together. For point-based GeoSources, the icons that represent the points also get closer together, but they don’t shrink (otherwise they would become too small to see). If we did nothing to prevent it, the icons would start overlapping each other to the point where it would become difficult to see the other information displayed on the Globe near them. Rather than let that happen, WorldView Studio automatically aggregates point-based data as you zoom out, replacing groups of densely packed icons with single larger icons. The larger icons are simply an indication that the points have been aggregated for your convenience. As you zoom in, the process is reversed and the large icons are replaced with the regular ones."
                },
                {
                    "question": "Why do some published GeoSources use icons that aren’t available in WorldView Studio?",
                    "answer": "WorldView Studio is a new product. Prior to its official release it was in development, including a Beta phase, and during that time GeoSources were published using styles that were available then, but which have since been modified and are no longer available. Rather than remove all of those published GeoSources we felt it was worth keeping them available despite some minor differences."
                }
            ]
        },
        {
            "heading": "Legend",
            "qaPairs": [
                {
                    "question": "Can I change the styles associated with the different values in the Legend?",
                    "answer": "If you click the style icon displayed beside a value (or range of values) in the Legend, you can select a different color to use for that value. Currently you cannot change the shape or size of icons for specific values."
                },
                {
                    "question": "Can I change the ranges of values that are displayed in the Legend?",
                    "answer": "No, at this time WorldView Studio automatically determines the ranges of values associated with each shade of color."
                }
            ]
        },
        {
            "heading": "Properties",
            "qaPairs": [
                {
                    "question": "Is it possible to get more information about a GeoSource, such as what the various Properties mean?",
                    "answer": "The person who publishes a GeoSource determines what information is made available. Sometimes you will find that the Description of the GeoSource (which you can access by opening the Edit GeoSource dialog) will include a link to a website where you can find additional information."
                }
            ]
        },
        {
            "heading": "Dashboard",
            "qaPairs": [
                {
                    "question": "Can I customize the way the Widgets appear in the Dashboard?",
                    "answer": "No, it is not currently possible to customize the appearance of the Widgets."
                }
            ]
        },
        {
            "heading": "Selections",
            "qaPairs": [
                {
                    "question": "Why doesn’t the Watershed tool selection match the watershed boundaries published in other places?",
                    "answer": "There are different definitions of what a watershed is. A couple of the more popular ones are:<ul><li>The area that drains down to a specified point.</li><li>The total area that collectively drains to a common point.</li></ul>The watershed tool in WorldView Studio conforms to the first definition, whereas watersheds with known published boundaries conform to the second of these definitions. While the definitions are different, in some cases they can result in the same selection. Specifically, if you click the watershed tool at a location that coincides with the lowest common point from a total watershed, the resulting selection will match both definitions. If you click any other point, the resulting selection will be a subset of the total watershed.<br>Note that our watershed selection is the result of an algorithm based on the available elevation data. If you clicked the same location using two different elevation GeoSources, you would get two selections that are similar but slightly different."
                }
            ]
        },
        {
            "heading": "Styles",
            "qaPairs": [
                {
                    "question": "If I changed the style for a GeoSource to display all features with a uniform color, is there a way to get it back to using a gradient so each feature appears in a different color?",
                    "answer": "Yes. A gradient is the result of applying a style based on a specific Property (as opposed to applying a style to the entire GeoSource). If you have applied a uniform style to a GeoSource, you can change that to a gradient by right-clicking the Globe at a location where the GeoSource is being displayed, then click the ‘+’ icon beside the specific Property to which you want to apply the color gradient."
                }
            ]
        },
        {
            "heading": "WorldView Gallery",
            "qaPairs": [
                {
                    "question": "I can search and publish from WorldView Studio. Why do I need to use WorldView Gallery?",
                    "answer": "Conceptually, WorldView Gallery is a place for browsing through the Maps and GeoSources that have been published, whether by large content providers or individual users. Although you can publish to WorldView Gallery from WorldView Studio, and can search for published content from WorldView Studio, the experiences are somewhat different. If the functionality available from WorldView Studio meets your personal requirements you may not care about using WorldView Gallery. The typical WorldView Gallery user is interested in sharing their information, and wants to take advantage of the extra functionality to make their data more desirable."
                }
            ]
        }
    ];

    $scope.makeSelection = function (sectionIndex, questionIndex) {
        if ($scope.selectedHeading === sectionIndex && $scope.selectedQuestion === questionIndex) {
            // toggle selection off
            $scope.selectedHeading = undefined;
            $scope.selectedQuestion = undefined;
            return;
        }
        $scope.selectedHeading = sectionIndex;
        $scope.selectedQuestion = questionIndex;
    }
});