###############################################################################
#
#   Class: NaturalDocs::Languages::Augeas
#
###############################################################################
#
#   A subclass to handle the language variations of Tcl.
#
###############################################################################

# This file is part of Natural Docs, which is Copyright (C) 2003-2008 Greg Valure
# Natural Docs is licensed under the GPL

use strict;
use integer;

package NaturalDocs::Languages::Augeas;

use base 'NaturalDocs::Languages::Simple';


my $pastFirstLet;
my $pastFirstTest;


sub OnCode {
   my ($self, @params) = @_;

   $pastFirstLet  = 0;
   $pastFirstTest = 0;

   return $self->SUPER::OnCode(@params);
};


# Override NormalizePrototype and ParsePrototype
sub NormalizePrototype {
   my ($self, $prototype) = @_;
   return $prototype;
}

sub ParsePrototype {
   my ($self, $type, $prototype) = @_;

   my $object = NaturalDocs::Languages::Prototype->New($prototype);
   return $object;
}



#
#   Function: OnPrototypeEnd
#
#   Tcl's function syntax is shown below.
#
#   > proc [name] { [params] } { [code] }
#
#   The opening brace is one of the prototype enders.  We need to allow the first opening brace because it contains the
#   parameters.
#
#   Also, the parameters may have braces within them.  I've seen one that used { seconds 20 } as a parameter.
#
#   Parameters:
#
#       type - The <TopicType> of the prototype.
#       prototypeRef - A reference to the prototype so far, minus the ender in dispute.
#       ender - The ender symbol.
#
#   Returns:
#
#       ENDER_ACCEPT - The ender is accepted and the prototype is finished.
#       ENDER_IGNORE - The ender is rejected and parsing should continue.  Note that the prototype will be rejected as a whole
#                                  if all enders are ignored before reaching the end of the code.
#       ENDER_ACCEPT_AND_CONTINUE - The ender is accepted so the prototype may stand as is.  However, the prototype might
#                                                          also continue on so continue parsing.  If there is no accepted ender between here and
#                                                          the end of the code this version will be accepted instead.
#       ENDER_REVERT_TO_ACCEPTED - The expedition from ENDER_ACCEPT_AND_CONTINUE failed.  Use the last accepted
#                                                        version and end parsing.
#
sub OnPrototypeEnd {
   my ($self, $type, $prototypeRef, $ender) = @_;

   if ($ender eq "\n") {
      return ::ENDER_ACCEPT_AND_CONTINUE();
   } elsif ( ($type eq "augeasvariable" || $type eq "augeaslens" || $type eq "augeastest") &&
              $ender eq "let" &&
              (!$pastFirstLet || $$prototypeRef =~ /\=[ \t\r\n]*$/
                              || $$prototypeRef =~ /in[ \t\r\n]+$/) ) {
      $pastFirstLet = 1;
      return ::ENDER_IGNORE();
   } elsif ( ($type eq "augeasvariable" || $type eq "augeaslens" || $type eq "augeastest") &&
              $ender eq "test" &&
              (!$pastFirstTest || $$prototypeRef =~ /\=[ \t\r\n]*$/
                               || $$prototypeRef =~ /in[ \t\r\n]+$/) ) {
      $pastFirstTest = 1;
      return ::ENDER_IGNORE();
   } else {
      return ::ENDER_ACCEPT();
   };
};


1;
