require File.join( File.dirname(__FILE__), 'lib_finder.rb' )

module RestinioZlibFind

  def self.find_libs( libs )

    lib_dirs = ENV[ "LIB" ]
    if lib_dirs

      dirs_to_look = lib_dirs.split( ';' ).select{|p| p != "" }

      dir = dirs_to_look.find_index{|d| RestinioLibFinder.all_files_exist( d, libs ) }
    end

    if dir
      libs
    else
      []
    end
  end

  def self.get_libs_vc
    find_libs( [ 'z.lib' ] )
  end

  def self.get_libs_linux
    [ 'z' ]
  end

  def self.get_libs_names_linux
    [ 'libz.a' ]
  end

  def self.get_libs_mingw
    [ 'z' ]
  end

  def self.get_libs_names_mingw
    [ 'libz.a' ]
  end

  def self.get_libs( toolset )
    if 'mswin' == toolset.tag( 'target_os' )
        if 'vc' == toolset.name
          get_libs_vc
        elsif 'gcc' == toolset.name
          get_libs_mingw
        end
    else
      get_libs_linux
    end
  end

  def self.get_libs_names( toolset )
    if 'mswin' == toolset.tag( 'target_os' )
        if 'vc' == toolset.name
          get_libs_vc
        elsif 'gcc' == toolset.name
          get_libs_names_mingw
        end
    else
      get_libs_names_linux
    end
  end

  def self.get_system_zlib( toolset )
    libraries = get_libs_names( toolset )

    if !libraries.empty? && RestinioLibFinder.check_libs_available( toolset, libraries )
      get_libs( toolset )
    else
      nil
    end
  end

end #module RestinioZlibFind
