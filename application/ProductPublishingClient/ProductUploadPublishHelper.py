# This file is used on the build machine for automated uploading and publishing of a built Pyxis product
# It is intended to be run as the last step of the product build
# The script may be extended to perform other operations

import os
import sys

def print_help():
    
    print 'Sample usage:'
    print
    print 'ProductUploadPublishHelper.py <username> <password> <env> <system_tag> <product_type> <product_version> <product_dir> <storage_client> <publishing_client>'
    print
    print 'Arguments:'
    print '\t<username>             username to login to the publishing server'
    print '\t<password>             The password'
    print '\t<env>                  Dev | Live'
    print '\t<system_tag>           Development | Production'
    print '\t<product_name>         The general name of the product'
    print '\t<product_version>      The version of the product'
    print '\t<product_dir>          The directory to upload'
    print '\t<storage_client>       Path to the StorageClient executable'
    print '\t<publishing_client>    Path to the ProductPublishingClient executable'

def main():
    
    # Command line arguments
    args = sys.argv
    if (10 != len(args)):
        print 'ProductUploadPublishHelper.py: too few arguments provided\n'
        print_help()
        return -1

    # Note: args[0] is the script path
    username = args[1]
    password = args[2]
    environment = args[3]
    system_tag = args[4]
    product_type = args[5]
    product_version = args[6]
    product_dir = args[7]
    storage_client = args[8]
    publishing_client = args[9]

    # Upload the artifacts to the pyxis storage
    print 'Uploading artifacts...'
    upload_cmd = ''
    upload_cmd += storage_client
    upload_cmd += ' UploadDirectory'
    upload_cmd += ' -dir='
    upload_cmd += '"' + product_dir + '"'
    print upload_cmd
    upload_pipe = os.popen(upload_cmd)
    key = ''
    # Check the output
    for l in upload_pipe:
        # The last line is the Product Storage Key
        print l
        if 0 != len(l):
            key = l
    upload_pipe.close()
    print 'Artifacts uploaded.'

    # Ensure character escaping in the storage key
    key = key.replace('"', '\\"')
    key = key.replace('\n', '')
    # Verify that it's actually the key
    if (0 == len(key)) or ('{' != key[0]) or ('}' != key[-1]):
        print 'Failed to retrieve the storage key'
        return -1

    # Publish the uploaded product
    print 'Publishing the product...'
    publish_cmd = ''
    publish_cmd += publishing_client
    publish_cmd += ' -POST'
    publish_cmd += ' -u=' + username
    publish_cmd += ' -p=' + password
    publish_cmd += ' -env=' + environment
    publish_cmd += ' -ProductType=' + product_type
    publish_cmd += ' -ProductVersion=' + product_version
    publish_cmd += ' -key=' + key
    publish_cmd += ' -SystemTag=' + system_tag
    success = True
    print publish_cmd
    publish_pipe = os.popen(publish_cmd)
    # Verify the upload operation result
    # Nothing shoul've been printed out
    for l in publish_pipe:
        print l
        if 0 != len(l.replace('\n', '')):
            success = False
    publish_pipe.close()
    if (success):
        print 'Product published.'
        return 0
    else:
        print 'Operation failed'
        return -1

if __name__ == "__main__":
    main()
