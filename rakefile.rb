myDir = File.dirname(File.expand_path(__FILE__));
require "#{myDir}/../build-options.rb";

module Rakish

Rakish::CppProject.new(
	:name 			=> "artd-jlib-util",
	:package 		=> "artd",
	:dependsUpon 	=> [ "../artd-lib-logger",
	                     "../artd-jlib-base"
	                   ]
) do

	cppDefine('BUILDING_artd_jlib_util');
		
	addPublicIncludes('include/artd/*.h');

    addSourceFiles(
        './Crc32.cpp',
        './ObjLinkedList.cpp',
        './TimingContext.cpp'
    );

    setupCppConfig :targetType =>'DLL' do |cfg|
    end
end

end # module Rakish

