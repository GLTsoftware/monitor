#!/usr/bin/perl
while(1) {
$utc = `/application/bin/value -v utc_hours`;
chomp($utc);
$az = `/application/bin/value -v actual_az_deg`;
chomp($az);
$el = `/application/bin/value -v actual_el_deg`;
chomp($el);
$azerr = `/application/bin/value -v az_tracking_error`;
chomp($azerr);
$elerr = `/application/bin/value -v el_tracking_error`;
chomp($elerr);
print "$utc $az $el $azerr $elerr\n";
sleep(1);
}
