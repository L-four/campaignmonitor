Campaign Monitor
================

----------------
Description
----------------
This module integrates the mailing list service Campaign Monitor. http://www.campaignmonitor.com

Campaign Monitor is a mailing list and newsletter service useful for web and
design professionals to manage newsletters and campaigns for your clients. The
client can then login through a reports interface to view the detailed reports
of their campaign.

TO USE THIS MODULE YOU MUST HAVE AN ACCOUNT WITH CAMPAIGNMONITOR.COM

This module adds a block that allows people to subscribe to a Campaign Monitor
list through the API. It also adds a page that lists past campaigns.

This module was developed by T-Rex Art: http://www.trexart.com.au

------------
Installation
------------
- At this time the module uses the built in libraries of PHP5 for he SOAP calls
so it is not available for use with PHP4. If you find there are errors, make
sure that your PHP installation has SOAP enabled.

- Create a folder in your modules directory called campaignmonitor and put the
module's files in this directory

- Enable the module on the Modules admin page:
    Administer > Site building > Modules

-------------
Configuration
-------------

Make sure to have your API Key and Client Id.
To retrieve these values follow the instructions here:
http://www.campaignmonitor.com/api/required.aspx

This module has been updated to work with the new combined Campaign Monitor and
MailBuild service, so please make sure you have updated your API keys if needed.

Enter these into the module's settings page. You will then be able to choose if
you wish all lists to be available on the site, or if you want to just select
certain lists to display on the site.

It is encouraged for you to spend some time setting up your campaign monitor
account before diving in with this module.

NOTE: There is a setting for each list in Campaign Monitor where if you
un-subscribe from one list, you are un-subscribed from all lists. The default
for this is to un-subscribe from all. This makes it so if in the 'My Newsletters'
area, a user un-subscribes from one list, they will be un-subscribed from all.
I recommend turning off this feature in the Campaign Monitor admin.

------------
Future plans
------------

I hope to integrate more of the API into this module in the future.

- Custom fields retrieved automatically.
- I will gladly take any suggestions.