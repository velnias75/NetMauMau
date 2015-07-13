####################################################################################################
#
# Copyright 2015 by Heiko Schäfer <heiko@rangun.de>
#
# This file is part of NetMauMau.
#
# NetMauMau is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# NetMauMau is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with NetMauMau.  If not, see <http://www.gnu.org/licenses/>.
#
####################################################################################################

match($0, /^NAME\(([^\)]+)\)/, m) { name = m[1]; }

END {

	print "/*";
	print " * Copyright " strftime("%Y") " by Heiko Schäfer <heiko@rangun.de>";
	print " *";
	print " * This file is part of NetMauMau.";
	print " *";
	print " * NetMauMau is free software: you can redistribute it and/or modify";
	print " * it under the terms of the GNU Lesser General Public License as";
	print " * published by the Free Software Foundation, either version 3 of";
	print " * the License, or (at your option) any later version.";
	print " *";
	print " * NetMauMau is distributed in the hope that it will be useful,";
	print " * but WITHOUT ANY WARRANTY; without even the implied warranty of";
	print " * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the";
	print " * GNU Lesser General Public License for more details.";
	print " *";
	print " * You should have received a copy of the GNU Lesser General Public License";
	print " * along with NetMauMau.  If not, see <http://www.gnu.org/licenses/>.";
	print " */\n";

	print "#ifndef NETMAUMAU_AI_" toupper(name) "_CAI_H";
	print "#define NETMAUMAU_AI_" toupper(name) "_CAI_H\n";

	print "#include \"abstractcondition.h\"          // for AbstractCondition\n";

	print "namespace NetMauMau {\n";

	print "namespace AI {\n";

	print "class " name " : public AbstractCondition {";
	print "\tDISALLOW_COPY_AND_ASSIGN(" name ")";
	print "public:";
	print "\t" name "();";
	print "\tvirtual ~" name "() _CONST;\n";

	print "\tvirtual IActionPtr perform(const IAIState &state, const Player::IPlayer::CARDS &cards) const;\n";

	print "#if defined(TRACE_AI) && !defined(NDEBUG)";
	print "protected:";
	print "\tinline virtual std::string traceLog() const {";
	print "\t\treturn \"" name "\";";
	print "\t}";
	print "#endif";
	print "};\n";

	print "}\n";

	print "}\n";

	print "#endif /* NETMAUMAU_AI_" toupper(name) "_CAI_H */";
}
