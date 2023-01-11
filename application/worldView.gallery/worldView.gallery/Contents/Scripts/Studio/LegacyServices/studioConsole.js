app.service('studioConsole', function ($document) {
    var wrapper = [
    '<div class="console-studio">',
        '<div class="console-toggle"><i class="fa fa-align-left"></i></div>',
        '<div class="console-content">',
            '<a class="console-exit" href="">Close</a>',
        '</div>',
    '</div>'];

    var studioConsole = {};
    var docBody = $document.find('body');

    //http://stackoverflow.com/questions/11616630/json-stringify-avoid-typeerror-converting-circular-structure-to-json
    JSON.stringifyOnce = function(obj, replacer, indent){
        var printedObjects = [];
        var printedObjectKeys = [];

        function printOnceReplacer(key, value){
            if ( printedObjects.length > 2000){ // browsers will not print more than 20K, I don't see the point to allow 2K.. algorithm will not be fast anyway if we have too many objects
            return 'object too long';
            }
            var printedObjIndex = false;
            printedObjects.forEach(function(obj, index){
                if (obj === value) {
                    printedObjIndex = index;
                }
            });

            if (key == '') { //root element
                printedObjects.push(obj);
                printedObjectKeys.push("root");
                return value;
            }

            else if (printedObjIndex + "" != "false" && typeof(value) == "object" ){
                if (printedObjectKeys[printedObjIndex] == "root"){
                    return "(pointer to root)";
                } else {
                    return "(see " + ((!!value && !!value.constructor) ? value.constructor.name.toLowerCase()  : typeof(value)) + " with key " + printedObjectKeys[printedObjIndex] + ")";
                }
            } else {

                var qualifiedKey = key || "(empty key)";
                printedObjects.push(value);
                printedObjectKeys.push(qualifiedKey);
                if (replacer) {
                    return replacer(key, value);
                } else {
                    return value;
                }
            }
        }
        return JSON.stringify(obj, printOnceReplacer, indent);
    };

    studioConsole.log = function() {
        var scaffold = [];
        var $console = $('.console-studio');
        
        if(!arguments) {
            return;
        }
       
        if (!$console.length) {
            docBody.append(wrapper.join(''));
            $console = docBody.find('.console-studio');
        }

        angular.forEach(arguments, function(argument, key) {
            var value = argument;
            var isType = 'string';

            if (value === null || !value) {
                scaffold.push('<div class="console-log">Value is Null</div>'); 
            } else {
                if (angular.isObject(argument)) {
                    isType = 'object';
                    value = JSON.stringifyOnce(argument, false, 4);
                }

                if (angular.isArray(argument)) {
                    isType = 'array';
                }

                if (angular.isFunction(argument)) {
                    isType = 'function';
                    value = argument();
                }

                if (angular.isNumber(argument)) {
                    isType = 'number';
                }

                if (typeof value === 'string' && value.substring(0, 2) === '##') {
                    scaffold.push('<div class="console-log console-header">' + value + '</div>');  
                } else {
                    scaffold.push('<div class="console-log ' + isType + '">' + value + '</div>');  
                } 
            }

        });

        $console.find('.console-exit').off();
        $console.find('.console-toggle').off();

        $console.find('.console-exit').on('click', function (e) {
            e.preventDefault();
            $console.remove();
        });

        $console.find('.console-toggle').on('click', function (e) {
            e.preventDefault();

            if ($console.hasClass('is-hidden')) {
                $console.removeClass('is-hidden');
            } else {
                $console.addClass('is-hidden');
            }
            
        });

        $console.find('.console-content').append(scaffold.join(''));

    }

    return studioConsole;
});
